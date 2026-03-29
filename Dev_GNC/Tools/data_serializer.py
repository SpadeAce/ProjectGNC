"""
data_serializer.py
------------------
protoc가 생성한 _pb2.py 모듈을 동적으로 import하고,
데이터 시트 행을 proto 메시지로 직렬화하여 .bytes 파일로 저장한다.

각 데이터 시트(MessageName)에 대해:
  1. {MessageName}Table 메시지를 생성한다.
  2. 각 행을 {MessageName} 메시지로 변환하여 Table에 추가한다.
  3. SerializeToString()으로 직렬화 후 output/bytes/{MessageName}.bytes에 저장한다.
"""

from __future__ import annotations

import importlib
import sys
from pathlib import Path
from typing import Any

from excel_parser import ExcelData, FieldDef


# ---------------------------------------------------------------------------
# Dynamic _pb2.py import
# ---------------------------------------------------------------------------

def _import_pb2(tool_dir: Path, package_name: str):
    """
    protoc가 생성한 {package_name}_pb2.py 모듈을 동적으로 import한다.

    output/py/ 디렉터리를 sys.path에 추가한 후 importlib으로 로드한다.
    동일 프로세스에서 재실행될 경우를 대비해 sys.modules 캐시를 제거한다.

    Args:
        tool_dir:     Tool 디렉터리 경로
        package_name: config의 package_name (예: "GameData")

    Returns:
        로드된 _pb2 모듈 객체

    Raises:
        ImportError: 모듈을 찾을 수 없거나 import 실패 시
    """
    py_out_dir = tool_dir / "output" / "py"
    py_out_str = str(py_out_dir.resolve())

    if py_out_str not in sys.path:
        sys.path.insert(0, py_out_str)

    module_name = f"{package_name}_pb2"

    # 재실행 시 캐시된 이전 모듈 제거
    if module_name in sys.modules:
        del sys.modules[module_name]

    try:
        module = importlib.import_module(module_name)
    except ModuleNotFoundError as e:
        raise ImportError(
            f"_pb2 모듈을 import할 수 없습니다: '{module_name}'\n"
            f"찾은 경로: {py_out_dir}\n"
            f"오류: {e}\n"
            "protoc 실행(cs_generator.run_protoc)이 성공적으로 완료되었는지 확인하세요."
        ) from e

    return module


# ---------------------------------------------------------------------------
# Enum name → number lookup
# ---------------------------------------------------------------------------

# {EnumName: {ValueName: ValueNumber}}
_EnumLookup = dict[str, dict[str, int]]


def _build_enum_lookup(enums) -> _EnumLookup:
    """
    ExcelData.enums로부터 이름 → 번호 룩업 테이블을 생성한다.

    Returns:
        {EnumName: {ValueName: ValueNumber}}
    """
    return {
        enum_name: {ev.value_name: ev.value_number for ev in values}
        for enum_name, values in enums.items()
    }


def _is_numeric_string(s: str) -> bool:
    """문자열이 숫자로 변환 가능한지 확인한다."""
    try:
        float(s.strip())
        return True
    except ValueError:
        return False


# ---------------------------------------------------------------------------
# Value coercion
# ---------------------------------------------------------------------------

def _coerce_scalar(value: Any, field_type: str, enum_lookup: _EnumLookup | None = None) -> Any:
    """
    단일 셀 값을 proto 필드 타입에 맞게 변환한다.

    enum 타입의 경우:
      - 숫자 / 숫자 문자열: int로 직접 변환
      - ValueName 문자열(예: "WARRIOR"): enum_lookup에서 번호로 변환

    Args:
        value:       Excel 셀의 raw 값
        field_type:  FieldDef.field_type 문자열
        enum_lookup: _build_enum_lookup() 결과 (None이면 숫자 변환만 시도)

    Returns:
        변환된 Python 값
    """
    if field_type in ("int32", "int64", "uint32", "uint64", "sint32", "sint64"):
        return int(float(str(value)))

    if field_type.startswith("enum:"):
        enum_name = field_type[5:]
        s = str(value).strip()
        # 숫자 문자열 또는 숫자 타입이면 그대로 int 변환
        if isinstance(value, (int, float)) or _is_numeric_string(s):
            return int(float(s))
        # ValueName 문자열 → 번호 룩업
        if enum_lookup and enum_name in enum_lookup:
            name_map = enum_lookup[enum_name]
            if s in name_map:
                return name_map[s]
            valid = sorted(name_map.keys())
            raise ValueError(
                f"enum '{enum_name}'에 '{s}' 값이 없습니다. "
                f"유효한 이름: {valid}"
            )
        raise ValueError(
            f"enum '{enum_name}'을 찾을 수 없어 '{s}'를 변환할 수 없습니다."
        )

    # asset:X / classref:X 타입 — UE5 에셋 경로 문자열
    if field_type.startswith("asset:") or field_type.startswith("classref:"):
        return str(value)

    if field_type in ("float", "double"):
        return float(value)

    if field_type == "bool":
        if isinstance(value, bool):
            return value
        s = str(value).strip().lower()
        return s in ("true", "1", "yes")

    # string, bytes, 또는 알 수 없는 타입
    return str(value)


def _coerce_value(value: Any, fd: FieldDef, enum_lookup: _EnumLookup | None = None) -> Any:
    """
    셀 값을 필드 정의(rule 포함)에 따라 변환한다.

    repeated 필드: 쉼표 구분 문자열을 split하여 각 요소를 변환한 리스트 반환.
    optional 필드: 단일 값 반환. None이면 None 반환(proto3 기본값 사용).

    Args:
        value:       Excel 셀의 raw 값
        fd:          해당 필드의 FieldDef
        enum_lookup: enum 이름 → 번호 룩업 테이블

    Returns:
        변환된 값 또는 None
    """
    if value is None:
        return None

    if fd.rule == "repeated":
        if isinstance(value, list):
            # 다중 컬럼 방식: 파서가 이미 리스트로 수집한 값
            return [_coerce_scalar(v, fd.field_type, enum_lookup)
                    for v in value if v is not None]
        else:
            # 하위 호환: 쉼표 구분 문자열 방식
            raw_str = str(value)
            parts = [p.strip() for p in raw_str.split(",") if p.strip()]
            return [_coerce_scalar(p, fd.field_type, enum_lookup) for p in parts]

    return _coerce_scalar(value, fd.field_type, enum_lookup)


# ---------------------------------------------------------------------------
# IdAlias resolution
# ---------------------------------------------------------------------------

def _build_alias_lookup(excel_data: ExcelData) -> dict[str, int]:
    """
    idAlias 필드가 있는 모든 테이블의 alias를 통합한 글로벌 룩업을 생성한다.
    alias는 전체 테이블에 걸쳐 고유해야 한다 (validator에서 보장).

    Returns:
        {alias_string: id_int}  ← 테이블 구분 없이 단일 dict
    """
    lookup: dict[str, int] = {}
    for msg_name, fields in excel_data.schema.items():
        has_alias = any(f.field_name == "idAlias" for f in fields)
        if not has_alias or msg_name not in excel_data.data:
            continue
        for row in excel_data.data[msg_name]:
            alias_val = row.get("idAlias")
            id_val    = row.get("id")
            if alias_val is not None and id_val is not None:
                lookup[str(alias_val).strip()] = int(float(str(id_val)))
    return lookup


def _resolve_id_alias_fields(
    row: dict,
    alias_lookup: dict[str, int],
) -> None:
    """
    row 내의 {X}IdAlias 필드를 찾아 {X}Id를 해석하여 row에 주입한다. (in-place)

    글로벌 alias 풀에서 직접 탐색하므로 필드명의 prefix와 테이블명이 일치하지 않아도 된다.

    동작:
      - 필드명이 'idAlias'가 아니면서 'IdAlias'로 끝나는 경우를 처리한다.
      - 대응하는 '{X}Id' 필드 값이 이미 있으면 alias 무시.
      - 숫자 문자열이면 바로 ID로 처리한다.
      - 아니면 글로벌 alias 풀에서 해당 alias를 찾아 ID를 주입한다.
    """
    for field_name, value in list(row.items()):
        if field_name == "idAlias" or not field_name.endswith("IdAlias"):
            continue
        if value is None:
            continue

        id_field_name = field_name[:-len("Alias")]  # "effectIdAlias" → "effectId"

        # 대응 Id 필드가 이미 채워져 있으면 무시
        if row.get(id_field_name) is not None:
            continue

        # repeated 필드: 파서가 리스트로 반환한 경우 각 요소를 개별 해석
        if isinstance(value, list):
            resolved = []
            for item in value:
                if item is None:
                    continue
                item_s = str(item).strip()
                if _is_numeric_string(item_s):
                    resolved.append(int(float(item_s)))
                elif item_s in alias_lookup:
                    resolved.append(alias_lookup[item_s])
                else:
                    raise ValueError(
                        f"'{field_name}': alias '{item_s}'를 찾을 수 없습니다. "
                        f"유효한 alias: {sorted(alias_lookup.keys())}"
                    )
            row[id_field_name] = resolved
            continue

        s = str(value).strip()

        # 숫자면 바로 ID
        if _is_numeric_string(s):
            row[id_field_name] = int(float(s))
            continue

        # 글로벌 alias 풀에서 탐색
        if s not in alias_lookup:
            raise ValueError(
                f"'{field_name}': alias '{s}'를 찾을 수 없습니다. "
                f"유효한 alias: {sorted(alias_lookup.keys())}"
            )
        row[id_field_name] = alias_lookup[s]


# ---------------------------------------------------------------------------
# Serialization
# ---------------------------------------------------------------------------

def serialize_all(
    excel_data: ExcelData,
    config: dict,
    tool_dir: Path,
) -> list[str]:
    """
    모든 데이터 시트를 proto 메시지로 직렬화하고 .bytes 파일로 저장한다.

    Args:
        excel_data: parse_excel()로 반환된 데이터
        config:     config.yaml 설정 dict
        tool_dir:   Tool 디렉터리 경로

    Returns:
        생성된 .bytes 파일 경로 문자열 목록

    Raises:
        ImportError:  _pb2.py 모듈 import 실패 시
        AttributeError: pb2 모듈에서 클래스를 찾을 수 없을 때
        Exception:    직렬화 중 예외 발생 시
    """
    package_name = config["package_name"]
    pb2 = _import_pb2(tool_dir, package_name)

    # alias 및 enum 룩업 테이블 (모든 시트 공통, 직렬화 전 선행 빌드)
    alias_lookup = _build_alias_lookup(excel_data)
    enum_lookup  = _build_enum_lookup(excel_data.enums)

    bytes_out = tool_dir / "output" / "bytes"
    bytes_out.mkdir(parents=True, exist_ok=True)

    written: list[str] = []

    for msg_name, rows in excel_data.data.items():
        if msg_name not in excel_data.schema:
            print(f"  경고: 데이터 시트 '{msg_name}'에 해당하는 스키마가 없습니다. 건너뜁니다.")
            continue

        table_class_name = f"{msg_name}Table"
        item_class_name  = msg_name

        # pb2 모듈에서 클래스 조회
        try:
            table_class = getattr(pb2, table_class_name)
            item_class  = getattr(pb2, item_class_name)
        except AttributeError as e:
            raise AttributeError(
                f"pb2 모듈에서 클래스를 찾을 수 없습니다: {e}\n"
                f"_Schema에 '{msg_name}'이 정의되어 있고 protoc가 성공적으로 실행되었는지 확인하세요."
            ) from e

        schema_fields = {fd.field_name: fd for fd in excel_data.schema[msg_name]}
        table_msg = table_class()

        for row in rows:
            # {X}IdAlias 필드를 해석하여 {X}Id에 주입 (in-place)
            _resolve_id_alias_fields(row, alias_lookup)

            item_msg = item_class()

            for field_name, raw_value in row.items():
                if field_name not in schema_fields:
                    continue

                # {X}IdAlias 필드는 proto에 없으므로 직렬화 제외
                if field_name != "idAlias" and field_name.endswith("IdAlias"):
                    continue

                fd = schema_fields[field_name]
                coerced = _coerce_value(raw_value, fd, enum_lookup)

                if coerced is None:
                    # proto3 기본값 사용 — 별도 설정 불필요
                    continue

                if fd.rule == "repeated":
                    # repeated 필드는 extend()로 추가
                    getattr(item_msg, field_name).extend(coerced)
                else:
                    setattr(item_msg, field_name, coerced)

            table_msg.items.append(item_msg)

        # .bytes 파일로 저장
        out_path = bytes_out / f"{msg_name}.bytes"
        out_path.write_bytes(table_msg.SerializeToString())
        written.append(str(out_path))

    return written
