"""
ImportDataTables.py
-------------------
UE5 Editor Python Script: Tools/output/json/ 의 JSON 파일을 DataTable 에셋으로 임포트한다.

classref/asset 필드 보존:
  기존 DataTable이 있을 경우 C++ 헬퍼(UDataTableEditorHelper)로 JSON을 추출한 뒤,
  _table_meta.json에 정의된 classref/asset 필드의 기존 값을 우선 적용한다.
  에디터에서 수동 설정한 클래스 레퍼런스가 유지된다.

사용법 (UE5 에디터 내):
  1. Edit > Project Settings > Plugins > Python Editor Script Plugin 활성화
  2. Output Log 하단의 Cmd 입력창에서:
       py "C:/UnrealProjects/ProjectGNC/Dev_GNC/Tools/ImportDataTables.py"
     또는 Tools > Execute Python Script 에서 이 파일 선택

새 테이블 추가 시:
  TABLE_STRUCT_MAP 딕셔너리에 테이블명과 Row Struct명을 추가하면 된다.
"""

import unreal
import json
import os

# ---------------------------------------------------------------------------
# 설정
# ---------------------------------------------------------------------------

# JSON 파일 디렉터리 (이 스크립트 기준 상대경로)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
JSON_DIR = os.path.join(SCRIPT_DIR, "output", "json")
META_PATH = os.path.join(JSON_DIR, "_table_meta.json")

# DataTable 에셋 저장 경로 (Content 기준)
ASSET_DIR = "/Game/CardGame/DataTables"

# 테이블명 → Row Struct명 매핑
# JSON 파일명: DT_{테이블명}.json
# Row Struct: /Script/Dev_GNC.{Struct명}
TABLE_STRUCT_MAP = {
    "PawnData":        "PawnDataRow",
    "MonsterData":     "MonsterDataRow",
    "CardData":        "CardDataRow",
    "CardEffectData":  "CardEffectRow",
    "EquipmentData":   "EquipmentDataRow",
    "TileEntityData":  "TileEntityRow",
    "StagePresetData": "StagePresetRow",
    "StageRewardData": "StageRewardRow",
    "ShopData":        "ShopDataRow",
    "PawnGrowthData":  "PawnGrowthRow",
    "NamePresetData":  "NamePresetRow",
    "TextData":        "TextRow",
}

# ---------------------------------------------------------------------------
# 메타데이터
# ---------------------------------------------------------------------------

def _load_table_meta():
    """_table_meta.json에서 classref/asset 필드 목록을 로드한다."""
    if not os.path.exists(META_PATH):
        return {}
    try:
        with open(META_PATH, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return {}


# ---------------------------------------------------------------------------
# classref/asset 보존 (C++ 헬퍼 기반)
# ---------------------------------------------------------------------------

def _is_valid_ref_value(val):
    """UE5 에셋 경로로 유효한 값인지 판별한다."""
    if isinstance(val, list):
        return any(isinstance(v, str) and v.startswith("/") for v in val)
    if isinstance(val, str):
        return val.startswith("/")
    return False


def _export_existing_json(data_table):
    """
    C++ 헬퍼를 통해 기존 DataTable의 JSON을 추출한다.

    Returns:
        JSON 문자열 또는 None (실패 시)
    """
    try:
        json_str = unreal.DataTableEditorHelper.get_data_table_as_json(data_table)
        if json_str and len(json_str) > 2:  # "[]" 보다 긴 경우
            return json_str
    except Exception as e:
        unreal.log_warning(f"  [WARN] C++ 헬퍼 호출 실패: {e}")
    return None


def _merge_with_existing(new_json_str, existing_json_str, ref_fields):
    """
    기존 DataTable JSON에서 classref/asset 필드를 추출하여 새 JSON에 병합한다.
    기존 값이 유효한 UE5 에셋 경로이면 우선 적용한다.

    Returns:
        (병합된 JSON 문자열, 보존된 필드 수)
    """
    new_rows = json.loads(new_json_str)

    # 기존 데이터: {row_name: {lowercase_field: value}}
    existing_map = {}
    if existing_json_str:
        try:
            existing_rows = json.loads(existing_json_str)
            for row in existing_rows:
                name = row.get("Name", "")
                existing_map[name] = {k.lower(): v for k, v in row.items()}
        except Exception:
            pass

    preserve_count = 0
    for row in new_rows:
        row_name = row.get("Name", "")
        if row_name not in existing_map:
            continue  # 새 행 — 보존할 것 없음

        old = existing_map[row_name]
        for field in ref_fields:
            old_val = old.get(field.lower(), "")
            if _is_valid_ref_value(old_val):
                # 기존 값이 유효한 UE5 경로 → 기존 값 우선
                row[field] = old_val
                preserve_count += 1

    merged = json.dumps(new_rows, ensure_ascii=False, indent=2)
    return merged, preserve_count


# ---------------------------------------------------------------------------
# 유틸리티
# ---------------------------------------------------------------------------

def find_row_struct(struct_name):
    """UScriptStruct를 이름으로 검색한다."""
    full_path = f"/Script/Dev_GNC.{struct_name}"
    struct = unreal.find_object(None, full_path)
    if struct is None:
        # F prefix 포함 시도
        struct = unreal.find_object(None, f"/Script/Dev_GNC.F{struct_name}")
    return struct


def import_single_table(table_name, struct_name, table_meta):
    """단일 DataTable을 JSON에서 임포트/갱신한다."""
    json_path = os.path.join(JSON_DIR, f"DT_{table_name}.json")
    if not os.path.exists(json_path):
        unreal.log_warning(f"[SKIP] JSON not found: {json_path}")
        return False

    asset_path = f"{ASSET_DIR}/DT_{table_name}"

    # Row Struct 찾기
    row_struct = find_row_struct(struct_name)
    if row_struct is None:
        unreal.log_error(f"[ERROR] Row Struct not found: {struct_name}")
        return False

    # 기존 에셋 확인
    existing = unreal.EditorAssetLibrary.load_asset(asset_path)

    if existing is None:
        # 새로 생성
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        factory = unreal.DataTableFactory()
        factory.struct = row_struct
        data_table = asset_tools.create_asset(
            f"DT_{table_name}",
            ASSET_DIR,
            unreal.DataTable,
            factory,
        )
        if data_table is None:
            unreal.log_error(f"[ERROR] Failed to create: DT_{table_name}")
            return False
        unreal.log(f"  [NEW] DT_{table_name} created")
    else:
        data_table = existing
        unreal.log(f"  [UPDATE] DT_{table_name} exists, updating data")

    # JSON 읽기
    with open(json_path, "r", encoding="utf-8") as f:
        json_string = f.read()

    # classref/asset 필드 보존
    ref_fields = table_meta.get(table_name, [])
    if ref_fields and existing is not None:
        unreal.log(f"  [REF] classref/asset 필드 보존 시도: {ref_fields}")

        existing_json = _export_existing_json(data_table)
        if existing_json:
            json_string, preserve_count = _merge_with_existing(
                json_string, existing_json, ref_fields
            )
            if preserve_count > 0:
                unreal.log(f"  [REF] {preserve_count}개 필드 보존 완료")

                # 보존된 값을 JSON 파일에도 기록 (다음 파이프라인 실행 시 유지)
                try:
                    with open(json_path, "w", encoding="utf-8") as f:
                        f.write(json_string)
                    unreal.log(f"  [REF] JSON 파일 업데이트: {json_path}")
                except Exception as e:
                    unreal.log_warning(f"  [WARN] JSON 파일 쓰기 실패: {e}")
            else:
                unreal.log(f"  [REF] 보존할 유효한 기존 값 없음")
        else:
            unreal.log(f"  [REF] 기존 DataTable JSON 추출 실패 — 보존 건너뜀")

    # DataTable에 JSON 데이터 채우기
    result = unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(
        data_table, json_string
    )

    if not result:
        unreal.log_error(f"[ERROR] JSON import failed: DT_{table_name}")
        return False

    # 에셋 저장
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return True


# ---------------------------------------------------------------------------
# 메인
# ---------------------------------------------------------------------------

def main():
    unreal.log("=" * 60)
    unreal.log("  DataTable JSON Import")
    unreal.log("=" * 60)

    # JSON 디렉터리에서 자동 감지
    if not os.path.isdir(JSON_DIR):
        unreal.log_error(f"JSON directory not found: {JSON_DIR}")
        return

    # 메타데이터 로드
    table_meta = _load_table_meta()
    if table_meta:
        unreal.log(f"  [META] classref/asset 필드 정보 로드: {list(table_meta.keys())}")
    else:
        unreal.log(f"  [META] _table_meta.json 없음 — classref 보존 비활성")

    success_count = 0
    skip_count = 0
    fail_count = 0

    # 매핑된 테이블 임포트
    for table_name, struct_name in TABLE_STRUCT_MAP.items():
        result = import_single_table(table_name, struct_name, table_meta)
        if result:
            success_count += 1
        else:
            fail_count += 1

    # 매핑에 없는 JSON 파일 감지 (새 테이블 추가 알림)
    for filename in os.listdir(JSON_DIR):
        if not filename.startswith("DT_") or not filename.endswith(".json"):
            continue
        name = filename[3:-5]  # "DT_XxxData.json" → "XxxData"
        if name not in TABLE_STRUCT_MAP:
            unreal.log_warning(
                f"  [WARN] {filename} has no struct mapping in TABLE_STRUCT_MAP. Skipped."
            )
            skip_count += 1

    unreal.log("")
    unreal.log(f"  Result: {success_count} imported, {fail_count} failed, {skip_count} unmapped")
    unreal.log("=" * 60)


main()
