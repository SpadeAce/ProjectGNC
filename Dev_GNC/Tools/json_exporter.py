"""
json_exporter.py
----------------
Protobuf .bytes 파일을 UE5 UDataTable용 JSON으로 변환한다.

main.py Step 8에서 호출되며, 독립 실행도 가능하다.
"""

from __future__ import annotations

import importlib
import json
import sys
from pathlib import Path

from google.protobuf.json_format import MessageToDict


# ---------------------------------------------------------------------------
# UE5 UENUM 이름 매핑 (proto int → UE5 enum value name)
# ---------------------------------------------------------------------------

ENUM_MAPS: dict[str, dict[int, str]] = {
    "ClassType": {
        0: "None", 1: "Scout", 2: "Assault", 3: "Heavy", 4: "Specialist",
    },
    "ItemType": {
        0: "None", 1: "Card", 2: "Buff", 3: "Equipment", 4: "Consume", 5: "Goods",
    },
    "TargetType": {
        0: "None", 1: "Self", 2: "Enemy", 3: "Ground", 4: "Ally", 5: "EmptyGround",
    },
    "CardType": {
        0: "None", 1: "Stable", 2: "Supply", 3: "Consume",
    },
    "CardEffectType": {
        0: "None", 1: "Damage", 2: "Heal", 3: "Shield",
        4: "RestoreAction", 5: "Reload", 6: "DrawCard",
        7: "BuffAttack", 8: "BuffArmor", 9: "BuffMovement",
        10: "DebuffAttack", 11: "DebuffArmor", 12: "DebuffMovement",
    },
    "TileEntityType": {
        0: "None", 1: "Actor", 2: "WallHalf", 3: "WallFull",
    },
    "StatusType": {
        0: "None", 1: "Atk", 2: "Def", 3: "HP", 4: "Shield",
        5: "Movement", 6: "Range", 7: "Accuracy", 8: "CardCap",
        9: "AmmoCap", 10: "EnergyCap",
    },
    "EquipSlotType": {
        0: "None", 1: "WeaponMain", 2: "WeaponSub",
        3: "Armor", 4: "Helmet", 5: "Tool",
    },
}

# proto 필드명 → enum 타입명 매핑
FIELD_ENUM_TYPE: dict[str, str] = {
    "classType": "ClassType",
    "type": "CardType",
    "target": "TargetType",
    "effectType": "CardEffectType",
    "entityType": "TileEntityType",
    "slot": "EquipSlotType",
    "statusType": "StatusType",
}


# ---------------------------------------------------------------------------
# 메시지 → dict 변환
# ---------------------------------------------------------------------------

def _msg_to_dict(msg) -> dict:
    """proto 메시지를 UE5 DataTable JSON용 dict로 변환한다."""
    d = MessageToDict(
        msg,
        preserving_proto_field_name=True,
        always_print_fields_with_no_presence=True,
        use_integers_for_enums=True,
    )

    # enum 정수값 → UE5 UENUM 이름 변환
    for field_name, enum_type_name in FIELD_ENUM_TYPE.items():
        if field_name not in d:
            continue
        emap = ENUM_MAPS.get(enum_type_name)
        if not emap:
            continue
        value = d[field_name]
        if isinstance(value, list):
            d[field_name] = [emap.get(v, str(v)) for v in value]
        else:
            d[field_name] = emap.get(value, str(value))

    return d


# ---------------------------------------------------------------------------
# 공개 API
# ---------------------------------------------------------------------------

def export_bytes_to_json(
    bytes_dir: Path,
    pb2_dir: Path,
    output_dir: Path,
    package_name: str,
    table_names: list[str],
) -> list[str]:
    """
    .bytes 파일들을 UE5 DataTable JSON으로 변환한다.

    Args:
        bytes_dir:    .bytes 파일 디렉터리
        pb2_dir:      _pb2.py 모듈 디렉터리
        output_dir:   JSON 출력 디렉터리
        package_name: proto 패키지명 (예: "GameData")
        table_names:  변환할 테이블명 리스트

    Returns:
        생성된 JSON 파일 경로 문자열 목록
    """
    # pb2 모듈 로드
    pb2_str = str(pb2_dir.resolve())
    if pb2_str not in sys.path:
        sys.path.insert(0, pb2_str)

    module_name = f"{package_name}_pb2"
    if module_name in sys.modules:
        del sys.modules[module_name]
    pb2 = importlib.import_module(module_name)

    output_dir.mkdir(parents=True, exist_ok=True)
    written: list[str] = []

    for table_name in table_names:
        bytes_path = bytes_dir / f"{table_name}.bytes"
        if not bytes_path.exists():
            print(f"  [SKIP] {bytes_path.name} 파일이 없습니다.")
            continue

        table_class = getattr(pb2, f"{table_name}Table")
        table_msg = table_class()
        table_msg.ParseFromString(bytes_path.read_bytes())

        rows = []
        for item in table_msg.items:
            d = _msg_to_dict(item)
            row_name = d.get("idAlias") or f"Row_{d.get('id', 0)}"
            d["Name"] = row_name
            rows.append(d)

        out_path = output_dir / f"DT_{table_name}.json"
        out_path.write_text(
            json.dumps(rows, ensure_ascii=False, indent=2),
            encoding="utf-8",
        )
        written.append(str(out_path))
        print(f"  [OK]{out_path.name}: {len(rows)} rows")

    return written
