"""
validator.py
------------
Excel에서 파싱된 스키마와 데이터를 검증한다.

검증 항목:
  스키마: 필드 번호 중복, 예약 범위(19000-19999), rule 값, enum 타입 참조 유효성,
          enum의 0번 값 존재 여부(proto3 요건)
  데이터: 시트명-스키마 매핑, 컬럼명 일치, 셀 값 타입
"""

from __future__ import annotations

from typing import Any

from excel_parser import ExcelData, FieldDef


# ---------------------------------------------------------------------------
# Custom exception
# ---------------------------------------------------------------------------

class ValidationError(Exception):
    """검증 실패 시 발생하는 예외. 시트명과 행 번호를 포함한다."""

    def __init__(self, message: str, sheet: str = "", row: int = 0) -> None:
        self.sheet = sheet
        self.row = row
        if sheet and row:
            location = f"[{sheet} {row}행]"
        elif sheet:
            location = f"[{sheet}]"
        else:
            location = ""
        full_msg = f"{location} {message}".strip()
        super().__init__(full_msg)


# ---------------------------------------------------------------------------
# Type validators
# ---------------------------------------------------------------------------

# bool은 Python에서 int의 서브클래스이므로 반드시 bool을 먼저 체크해야 한다.
_SCALAR_TYPE_MAP: dict[str, tuple[str, callable]] = {
    "int32":  ("정수",   lambda v: isinstance(v, (int, float)) and not isinstance(v, bool)),
    "int64":  ("정수",   lambda v: isinstance(v, (int, float)) and not isinstance(v, bool)),
    "uint32": ("양의 정수", lambda v: isinstance(v, (int, float)) and not isinstance(v, bool) and float(v) >= 0),
    "uint64": ("양의 정수", lambda v: isinstance(v, (int, float)) and not isinstance(v, bool) and float(v) >= 0),
    "sint32": ("정수",   lambda v: isinstance(v, (int, float)) and not isinstance(v, bool)),
    "sint64": ("정수",   lambda v: isinstance(v, (int, float)) and not isinstance(v, bool)),
    "float":  ("실수",   lambda v: isinstance(v, (int, float)) and not isinstance(v, bool)),
    "double": ("실수",   lambda v: isinstance(v, (int, float)) and not isinstance(v, bool)),
    "string": ("문자열", lambda v: isinstance(v, str)),
    "bool":   ("불리언", lambda v: isinstance(v, bool)),
    "bytes":  ("문자열", lambda v: isinstance(v, str)),
}

_RESERVED_FIELD_NUMBER_MIN = 19000
_RESERVED_FIELD_NUMBER_MAX = 19999
_VALID_RULES = {"optional", "repeated"}


def _is_valid_cell(
    value: Any,
    field_type: str,
    enum_value_names: set[str] | None = None,
) -> bool:
    """
    셀 값이 해당 proto 타입에 적합한지 검사한다.

    enum:X 타입 허용 형식:
      - 정수 또는 실수 (ValueNumber 직접 입력): 1, 2.0
      - 숫자로 변환 가능한 문자열: "1", "2"
      - enum ValueName 문자열: "WARRIOR", "ARCHER"  ← enum_value_names 제공 시

    repeated 필드: 쉼표 구분 문자열 또는 단일 값을 허용 (요소별 검사 없음).
    """
    if field_type.startswith("enum:"):
        if isinstance(value, bool):
            return False
        # 숫자 타입
        if isinstance(value, (int, float)):
            return True
        if isinstance(value, str):
            s = value.strip()
            # 숫자 문자열
            try:
                int(float(s))
                return True
            except ValueError:
                pass
            # enum ValueName 문자열
            if enum_value_names and s in enum_value_names:
                return True
        return False

    # asset:X / classref:X 타입 — UE5 에셋 경로 문자열
    if field_type.startswith("asset:") or field_type.startswith("classref:"):
        return isinstance(value, str)

    if field_type in _SCALAR_TYPE_MAP:
        _, checker = _SCALAR_TYPE_MAP[field_type]
        return checker(value)

    # 알 수 없는 타입은 통과 (추후 protoc가 검증)
    return True


# ---------------------------------------------------------------------------
# Schema validation
# ---------------------------------------------------------------------------

def _validate_schema(excel_data: ExcelData) -> None:
    """스키마 정의의 유효성을 검사한다."""
    known_enums = set(excel_data.enums.keys())

    for msg_name, fields in excel_data.schema.items():
        seen_numbers: dict[int, str] = {}
        field_names = {f.field_name for f in fields}

        for fd in fields:
            # rule 검증
            if fd.rule not in _VALID_RULES:
                raise ValidationError(
                    f"알 수 없는 Rule '{fd.rule}'. "
                    f"허용값: {sorted(_VALID_RULES)}",
                    sheet="_Schema",
                )

            # 필드 번호 범위 검증
            if fd.field_number < 1:
                raise ValidationError(
                    f"{msg_name}.{fd.field_name}: 필드 번호는 1 이상이어야 합니다 "
                    f"(현재: {fd.field_number}).",
                    sheet="_Schema",
                )

            if _RESERVED_FIELD_NUMBER_MIN <= fd.field_number <= _RESERVED_FIELD_NUMBER_MAX:
                raise ValidationError(
                    f"{msg_name}.{fd.field_name}: 필드 번호 {fd.field_number}는 "
                    f"proto 예약 범위({_RESERVED_FIELD_NUMBER_MIN}-"
                    f"{_RESERVED_FIELD_NUMBER_MAX})에 속합니다.",
                    sheet="_Schema",
                )

            # 필드 번호 중복 검증
            if fd.field_number in seen_numbers:
                raise ValidationError(
                    f"{msg_name}: 필드 번호 {fd.field_number}가 중복됩니다 "
                    f"('{seen_numbers[fd.field_number]}' 와 '{fd.field_name}').",
                    sheet="_Schema",
                )
            seen_numbers[fd.field_number] = fd.field_name

            # enum 타입 참조 검증
            if fd.field_type.startswith("enum:"):
                enum_name = fd.field_type[5:]
                if enum_name not in known_enums:
                    raise ValidationError(
                        f"{msg_name}.{fd.field_name}: "
                        f"참조한 enum '{enum_name}'이 _Enum 시트에 정의되어 있지 않습니다.",
                        sheet="_Schema",
                    )

            # asset:X / classref:X 타입은 별도 참조 검증 없음 (UE5 에셋 경로)

            # {X}IdAlias 필드는 반드시 대응하는 {X}Id 필드가 같은 메시지에 있어야 한다
            if fd.field_name != "idAlias" and fd.field_name.endswith("IdAlias"):
                pair_name = fd.field_name[:-len("Alias")]  # "pawnIdAlias" → "pawnId"
                if pair_name not in field_names:
                    raise ValidationError(
                        f"{msg_name}.{fd.field_name}: "
                        f"대응하는 Id 필드 '{pair_name}'이 스키마에 없습니다. "
                        f"'{fd.field_name}'는 '{pair_name}'와 쌍으로 정의해야 합니다.",
                        sheet="_Schema",
                    )

            # 알 수 없는 스칼라 타입 경고 (오류는 protoc에서 발생)
            if (not fd.field_type.startswith("enum:")
                    and not fd.field_type.startswith("asset:")
                    and not fd.field_type.startswith("classref:")
                    and fd.field_type not in _SCALAR_TYPE_MAP):
                print(
                    f"  경고: {msg_name}.{fd.field_name}의 타입 '{fd.field_type}'은 "
                    "알 수 없는 proto 타입입니다."
                )


def _validate_enums(excel_data: ExcelData) -> None:
    """
    enum 정의를 검증한다.

    proto3 요건: 모든 enum은 첫 번째 값이 0이어야 한다.
    """
    for enum_name, values in excel_data.enums.items():
        numbers = [v.value_number for v in values]

        # 0번 값 존재 여부 (proto3 필수)
        if 0 not in numbers:
            raise ValidationError(
                f"enum '{enum_name}'에 값 번호 0이 없습니다. "
                "proto3에서는 모든 enum이 0번 값을 가져야 합니다.",
                sheet="_Enum",
            )

        # 중복 번호 검증
        if len(numbers) != len(set(numbers)):
            seen: dict[int, str] = {}
            for v in values:
                if v.value_number in seen:
                    raise ValidationError(
                        f"enum '{enum_name}': 값 번호 {v.value_number}가 중복됩니다 "
                        f"('{seen[v.value_number]}' 와 '{v.value_name}').",
                        sheet="_Enum",
                    )
                seen[v.value_number] = v.value_name


# ---------------------------------------------------------------------------
# Alias validation
# ---------------------------------------------------------------------------

def _validate_id_alias_uniqueness(excel_data: ExcelData) -> None:
    """
    모든 테이블의 idAlias 값이 전체적으로 고유한지 검증한다.

    글로벌 alias 풀을 사용하므로 서로 다른 테이블이라도 동일한 alias를 가질 수 없다.
    중복이 있으면 {X}IdAlias 입력 시 잘못된 테이블의 ID로 해석될 수 있다.
    """
    seen: dict[str, str] = {}  # alias_string → "MessageName N행"
    for msg_name, fields in excel_data.schema.items():
        if not any(f.field_name == "idAlias" for f in fields):
            continue

        rows = excel_data.data.get(msg_name, [])
        for row_idx, row in enumerate(rows, start=2):
            val = row.get("idAlias")
            if val is None:
                continue
            s = str(val).strip()
            if not s:
                continue
            if s in seen:
                raise ValidationError(
                    f"idAlias '{s}'가 전역 중복됩니다 "
                    f"(이미 {seen[s]}에서 사용 중).",
                    sheet=msg_name,
                    row=row_idx,
                )
            seen[s] = f"{msg_name} {row_idx}행"


# ---------------------------------------------------------------------------
# Data validation
# ---------------------------------------------------------------------------

def _build_enum_name_sets(
    schema_fields: list[FieldDef],
    enums: dict,
) -> dict[str, set[str]]:
    """
    스키마 필드 목록에서 enum 타입 필드별로 유효한 ValueName 집합을 반환한다.

    Returns:
        {field_name: {ValueName, ...}} — enum 필드만 포함
    """
    result: dict[str, set[str]] = {}
    for fd in schema_fields:
        if fd.field_type.startswith("enum:"):
            enum_name = fd.field_type[5:]
            if enum_name in enums:
                result[fd.field_name] = {ev.value_name for ev in enums[enum_name]}
    return result


def _validate_data_sheet(
    sheet_name: str,
    rows: list[dict],
    schema_fields: list[FieldDef],
    enums: dict,
) -> None:
    """단일 데이터 시트의 컬럼명과 셀 타입을 검증한다."""
    expected_columns = {fd.field_name for fd in schema_fields}
    field_map = {fd.field_name: fd for fd in schema_fields}
    enum_name_sets = _build_enum_name_sets(schema_fields, enums)

    if not rows:
        return

    # 컬럼명 집합 검증 (첫 번째 데이터 행의 키로 판단)
    actual_columns = set(rows[0].keys())
    missing = expected_columns - actual_columns
    extra   = actual_columns - expected_columns

    # {X}IdAlias가 시트에 있으면 대응하는 {X}Id는 생략 가능 (직렬화 시 자동 주입)
    alias_covered = {
        col[:-len("Alias")]  # "pawnIdAlias" → "pawnId"
        for col in actual_columns
        if col != "idAlias" and col.endswith("IdAlias")
    }
    missing -= alias_covered

    if missing:
        print(
            f"  경고: [{sheet_name}] 스키마에 있지만 시트에 없는 컬럼: {sorted(missing)} "
            "→ 기본값(proto3 zero value)으로 채워집니다."
        )
    if extra:
        raise ValidationError(
            f"시트에 있지만 스키마에 없는 컬럼: {sorted(extra)}",
            sheet=sheet_name,
        )

    # 셀 타입 검증 (2행부터 시작; 헤더는 파서에서 제거됨)
    for row_idx, row in enumerate(rows, start=2):
        for field_name, value in row.items():
            if field_name not in field_map:
                continue

            fd = field_map[field_name]

            # None 값은 proto3 기본값 처리 — 검증 스킵
            if value is None:
                continue

            if fd.rule == "repeated":
                if isinstance(value, list):
                    # 다중 컬럼 방식: 각 요소를 개별 검증
                    enum_value_names = enum_name_sets.get(field_name)
                    for elem_idx, elem in enumerate(value):
                        if elem is None:
                            continue
                        if not _is_valid_cell(elem, fd.field_type, enum_value_names):
                            if fd.field_type.startswith("enum:"):
                                valid_names = sorted(enum_value_names) if enum_value_names else []
                                type_hint = f"정수 또는 enum 이름({', '.join(valid_names)})"
                            else:
                                type_hint = _SCALAR_TYPE_MAP.get(fd.field_type, ("알 수 없음", None))[0]
                            raise ValidationError(
                                f"컬럼 '{field_name}'[{elem_idx}]: 값 {elem!r}이 타입 "
                                f"'{fd.field_type}'({type_hint})과 맞지 않습니다.",
                                sheet=sheet_name,
                                row=row_idx,
                            )
                # 단일 값(쉼표 구분 문자열 또는 스칼라) → 허용 (하위 호환)
                continue

            # optional 필드에 리스트가 들어온 경우 → 다중 컬럼 오용 오류
            if isinstance(value, list):
                raise ValidationError(
                    f"컬럼 '{field_name}': optional 필드에 여러 컬럼이 사용되었습니다. "
                    "다중 컬럼은 Rule이 'repeated'인 필드에만 사용할 수 있습니다.",
                    sheet=sheet_name,
                    row=row_idx,
                )

            enum_value_names = enum_name_sets.get(field_name)
            if not _is_valid_cell(value, fd.field_type, enum_value_names):
                if fd.field_type.startswith("enum:"):
                    enum_name = fd.field_type[5:]
                    valid_names = sorted(enum_value_names) if enum_value_names else []
                    type_hint = f"정수 또는 enum 이름({', '.join(valid_names)})"
                else:
                    type_hint = _SCALAR_TYPE_MAP.get(fd.field_type, ("알 수 없음", None))[0]
                raise ValidationError(
                    f"컬럼 '{field_name}': 값 {value!r}이 타입 "
                    f"'{fd.field_type}'({type_hint})과 맞지 않습니다.",
                    sheet=sheet_name,
                    row=row_idx,
                )


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def validate(excel_data: ExcelData) -> None:
    """
    ExcelData 전체를 검증한다.

    Args:
        excel_data: parse_excel()로 반환된 데이터

    Raises:
        ValidationError: 검증 실패 시. 시트명과 행 번호를 포함한다.
    """
    _validate_enums(excel_data)
    _validate_schema(excel_data)
    _validate_id_alias_uniqueness(excel_data)

    for sheet_name, rows in excel_data.data.items():
        if sheet_name not in excel_data.schema:
            raise ValidationError(
                f"데이터 시트 '{sheet_name}'에 해당하는 스키마가 _Schema에 없습니다.",
                sheet=sheet_name,
            )
        _validate_data_sheet(
            sheet_name,
            rows,
            excel_data.schema[sheet_name],
            excel_data.enums,
        )
