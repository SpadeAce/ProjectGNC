"""
proto_generator.py
------------------
ExcelData로부터 .proto 파일을 생성하고, 스키마 스냅샷을 관리한다.

주요 기능:
  - Jinja2 템플릿(templates/proto_template.j2)으로 .proto 파일 생성
  - 각 Message에 대해 {MessageName}Table 래퍼 메시지 자동 생성
  - 이전 실행의 스냅샷(output/schema_snapshot.json)과 비교하여 필드 번호 변경 경고
"""

from __future__ import annotations

import json
import re
from pathlib import Path

import jinja2

from excel_parser import ExcelData


# ---------------------------------------------------------------------------
# Schema snapshot
# ---------------------------------------------------------------------------

_SNAPSHOT_FILENAME = "schema_snapshot.json"


def load_snapshot(tool_dir: Path) -> dict:
    """
    이전 실행에서 저장한 스키마 스냅샷을 로드한다.

    Args:
        tool_dir: Tool 디렉터리 경로

    Returns:
        스냅샷 dict. 파일이 없으면 빈 dict.
    """
    snapshot_path = tool_dir / "output" / _SNAPSHOT_FILENAME
    if snapshot_path.exists():
        with open(snapshot_path, encoding="utf-8") as f:
            return json.load(f)
    return {}


def compare_snapshot(excel_data: ExcelData, snapshot: dict) -> list[str]:
    """
    현재 스키마와 이전 스냅샷을 비교하여 필드 번호 변경 경고 목록을 반환한다.

    새로 추가된 메시지나 필드는 경고하지 않는다.
    기존에 있던 필드의 번호가 바뀐 경우에만 경고한다.

    Args:
        excel_data: 현재 파싱된 ExcelData
        snapshot:   load_snapshot()으로 로드한 이전 스냅샷

    Returns:
        경고 메시지 문자열 목록
    """
    warnings: list[str] = []

    for msg_name, fields in excel_data.schema.items():
        if msg_name not in snapshot:
            continue  # 새 메시지 — 경고 없음

        old_field_map: dict[str, dict] = {
            f["field_name"]: f for f in snapshot[msg_name]
        }

        for fd in fields:
            if fd.field_name not in old_field_map:
                continue  # 새 필드 — 경고 없음

            old_number = old_field_map[fd.field_name]["field_number"]
            if old_number != fd.field_number:
                warnings.append(
                    f"  [경고] {msg_name}.{fd.field_name}: "
                    f"필드 번호가 {old_number} → {fd.field_number}로 변경되었습니다. "
                    "이미 직렬화된 데이터와 하위 호환성이 깨질 수 있습니다."
                )

    return warnings


def save_snapshot(excel_data: ExcelData, tool_dir: Path) -> None:
    """
    현재 스키마를 스냅샷 파일에 저장한다.

    Args:
        excel_data: 현재 파싱된 ExcelData
        tool_dir:   Tool 디렉터리 경로
    """
    snapshot = {
        msg_name: [
            {
                "field_name":   fd.field_name,
                "field_number": fd.field_number,
                "field_type":   fd.field_type,
            }
            for fd in fields
        ]
        for msg_name, fields in excel_data.schema.items()
    }

    snapshot_path = tool_dir / "output" / _SNAPSHOT_FILENAME
    snapshot_path.parent.mkdir(parents=True, exist_ok=True)
    with open(snapshot_path, "w", encoding="utf-8") as f:
        json.dump(snapshot, f, indent=2, ensure_ascii=False)


# ---------------------------------------------------------------------------
# Proto type / enum value name conversion
# ---------------------------------------------------------------------------

def _to_proto_type(field_type: str) -> str:
    """
    Excel 스키마의 FieldType을 proto 타입 문자열로 변환한다.

    'enum:PawnType'         → 'PawnType'
    'asset:TileMapPreset'   → 'string'   (UE5 에셋 경로)
    'int32'                 → 'int32'
    """
    if field_type.startswith("enum:"):
        return field_type[5:]
    if field_type.startswith("asset:") or field_type.startswith("classref:"):
        return "string"
    return field_type


def _is_ref_alias_field(field_name: str) -> bool:
    """
    툴 전용 alias 입력 필드인지 확인한다. (proto 제외 대상)

    '{X}IdAlias' 패턴의 필드는 데이터 입력 편의를 위한 helper 컬럼으로,
    직렬화 시 대응하는 '{X}Id' 필드로 해석된 후 버려진다.
    정확히 'idAlias'인 경우는 해당 테이블의 별칭 선언 필드이므로 proto에 포함한다.

    예: 'pawnIdAlias' → True (proto 제외)
        'cardEffectIdAlias' → True (proto 제외)
        'idAlias' → False (proto 포함 — 테이블 자체의 별칭)
    """
    return field_name != "idAlias" and field_name.endswith("IdAlias")


def _to_upper_snake(name: str) -> str:
    """
    PascalCase 또는 camelCase 문자열을 UPPER_SNAKE_CASE로 변환한다.

    proto3 enum 값 prefix 생성에 사용된다.
    예: PawnType → PAWN_TYPE, ItemType → ITEM_TYPE

    proto3 C++ 스코핑 규칙상 패키지 내 모든 enum 값이 고유해야 한다.
    prefix를 붙이면 protoc C# 생성기가 자동으로 prefix를 제거하므로
    C#에서는 PawnType.None, PawnType.Warrior 형태로 정상 접근 가능하다.
    """
    s = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", name)
    return s.upper()


# ---------------------------------------------------------------------------
# Proto file generation
# ---------------------------------------------------------------------------

def _ensure_output_dirs(tool_dir: Path) -> None:
    """output 하위 디렉터리들을 생성한다."""
    for sub in ("proto", "cs", "py", "bytes"):
        (tool_dir / "output" / sub).mkdir(parents=True, exist_ok=True)


def generate_proto(excel_data: ExcelData, config: dict, tool_dir: Path) -> Path:
    """
    ExcelData로부터 .proto 파일을 생성한다.

    Args:
        excel_data: 파싱된 Excel 데이터
        config:     config.yaml에서 로드한 설정 dict
        tool_dir:   Tool 디렉터리 경로 (templates/ 기준)

    Returns:
        생성된 .proto 파일의 절대 경로

    Raises:
        FileNotFoundError: 템플릿 파일이 없을 때
        jinja2.TemplateError: 템플릿 렌더링 오류
    """
    _ensure_output_dirs(tool_dir)

    template_dir = tool_dir / "templates"
    if not (template_dir / "proto_template.j2").exists():
        raise FileNotFoundError(
            f"템플릿 파일을 찾을 수 없습니다: {template_dir / 'proto_template.j2'}"
        )

    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(str(template_dir)),
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True,
    )
    template = env.get_template("proto_template.j2")

    # 템플릿 컨텍스트 구성
    # proto3 C++ 스코핑 규칙 대응: 패키지 내 enum 값 이름 충돌 방지를 위해
    # 각 enum 값에 'ENUM_NAME_' prefix를 자동으로 붙인다.
    # 예) PawnType.NONE → PAWN_TYPE_NONE, ItemType.NONE → ITEM_TYPE_NONE
    # protoc C# 생성기는 이 prefix를 자동 제거하므로 C#에서는 PawnType.None으로 접근.
    enum_context = {
        enum_name: [
            {
                "value_name":    v.value_name,
                "prefixed_name": f"{_to_upper_snake(enum_name)}_{v.value_name}",
                "value_number":  v.value_number,
                "comment":       v.comment,
            }
            for v in values
        ]
        for enum_name, values in excel_data.enums.items()
    }

    message_context = {
        msg_name: [
            {
                "field_name":   fd.field_name,
                "proto_type":   _to_proto_type(fd.field_type),
                "field_number": fd.field_number,
                "rule":         fd.rule,
                "comment":      fd.comment,
            }
            for fd in fields
            if not _is_ref_alias_field(fd.field_name)
        ]
        for msg_name, fields in excel_data.schema.items()
    }

    context = {
        "package_name": config["package_name"],
        "enums":        enum_context,
        "messages":     message_context,
    }

    rendered = template.render(**context)

    package_name = config["package_name"]
    proto_path = tool_dir / "output" / "proto" / f"{package_name}.proto"
    proto_path.write_text(rendered, encoding="utf-8")

    return proto_path
