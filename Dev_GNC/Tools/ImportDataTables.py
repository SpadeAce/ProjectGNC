"""
ImportDataTables.py
-------------------
UE5 Editor Python Script: Tools/output/json/ 의 JSON 파일을 DataTable 에셋으로 임포트한다.

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


def import_single_table(table_name, struct_name):
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

    # JSON 읽기 및 데이터 채우기
    with open(json_path, "r", encoding="utf-8") as f:
        json_string = f.read()

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

    success_count = 0
    skip_count = 0
    fail_count = 0

    # 매핑된 테이블 임포트
    for table_name, struct_name in TABLE_STRUCT_MAP.items():
        result = import_single_table(table_name, struct_name)
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
