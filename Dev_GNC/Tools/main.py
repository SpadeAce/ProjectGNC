"""
main.py
-------
Excel → Protobuf → UE5 DataTable JSON 자동화 파이프라인.

사용법:
  python main.py                          # 기본 실행 (config.yaml 사용)
  python main.py --config other.yaml      # 설정 파일 경로 오버라이드
  python main.py --skip-json              # JSON 변환 단계 건너뜀
  python main.py --schema-dir ./Proto/    # 스키마 디렉터리 오버라이드
  python main.py --data-dir ./Table/      # 데이터 디렉터리 오버라이드

파이프라인 단계:
  1. config.yaml 로드
  2. Excel 파싱
  3. 스키마 / 데이터 검증
  4. 스키마 스냅샷 비교 (필드 번호 변경 경고)
  5. .proto 파일 생성 + 스냅샷 저장
  6. protoc 실행 → _pb2.py 생성
  7. 데이터 직렬화 → .bytes 생성
  8. .bytes → UE5 DataTable JSON 변환
"""

from __future__ import annotations

import argparse
import sys
import traceback
from pathlib import Path

import yaml


TOOL_DIR = Path(__file__).parent.resolve()


# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------

def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Excel → Protobuf → UE5 DataTable JSON 파이프라인",
    )
    parser.add_argument(
        "--config", metavar="PATH",
        default=str(TOOL_DIR / "config.yaml"),
        help="설정 파일 경로 (기본값: ./config.yaml)",
    )
    parser.add_argument(
        "--skip-json", action="store_true",
        help="JSON 변환 단계를 건너뜀 (.bytes까지만 생성)",
    )
    parser.add_argument("--schema-dir", metavar="PATH")
    parser.add_argument("--data-dir", metavar="PATH")
    return parser.parse_args()


# ---------------------------------------------------------------------------
# Step helpers
# ---------------------------------------------------------------------------

def _step(num: int, total: int, desc: str) -> None:
    print(f"\n[{num}/{total}] {desc}")

def _ok(msg: str = "") -> None:
    if msg:
        print(f"  [OK] {msg}")


# ---------------------------------------------------------------------------
# Pipeline
# ---------------------------------------------------------------------------

def run_pipeline(args: argparse.Namespace) -> None:
    TOTAL_STEPS = 8

    # ------------------------------------------------------------------
    # Step 1: config.yaml 로드
    # ------------------------------------------------------------------
    _step(1, TOTAL_STEPS, "config.yaml 로드")
    config_path = Path(args.config)
    if not config_path.exists():
        raise FileNotFoundError(f"설정 파일을 찾을 수 없습니다: {config_path}")

    with open(config_path, encoding="utf-8") as f:
        config = yaml.safe_load(f)

    required_keys = ("protoc_path", "package_name")
    missing = [k for k in required_keys if k not in config]
    if missing:
        raise ValueError(f"config.yaml에 필수 키가 없습니다: {missing}")

    _ok(f"설정 로드 완료: {config_path}")
    print(f"  package_name : {config['package_name']}")
    print(f"  schema_dir   : {args.schema_dir or config.get('schema_dir')}")
    print(f"  data_dir     : {args.data_dir or config.get('data_dir')}")

    # ------------------------------------------------------------------
    # Step 2: Excel 파싱
    # ------------------------------------------------------------------
    _step(2, TOTAL_STEPS, "Excel 파싱 (멀티 파일 모드)")
    from excel_parser import parse_excel_multi

    raw_schema_dir = args.schema_dir or config.get("schema_dir")
    raw_data_dir   = args.data_dir   or config.get("data_dir")

    if not raw_schema_dir or not raw_data_dir:
        raise ValueError("config.yaml에 schema_dir과 data_dir이 필요합니다.")

    schema_dir = (TOOL_DIR / raw_schema_dir).resolve()
    data_dir   = (TOOL_DIR / raw_data_dir).resolve()
    excel_data = parse_excel_multi(schema_dir, data_dir)
    _ok(f"schema_dir: {schema_dir}")
    _ok(f"data_dir  : {data_dir}")

    print(f"  메시지     : {list(excel_data.schema.keys())}")
    print(f"  Enum       : {list(excel_data.enums.keys())}")
    print(f"  데이터 시트: {list(excel_data.data.keys())}")

    # ------------------------------------------------------------------
    # Step 3: 스키마 / 데이터 검증
    # ------------------------------------------------------------------
    _step(3, TOTAL_STEPS, "스키마 / 데이터 검증")
    from validator import validate

    validate(excel_data)
    _ok("검증 통과")

    # ------------------------------------------------------------------
    # Step 4: 스키마 스냅샷 비교
    # ------------------------------------------------------------------
    _step(4, TOTAL_STEPS, "스키마 스냅샷 비교")
    from proto_generator import load_snapshot, compare_snapshot, save_snapshot, generate_proto

    snapshot = load_snapshot(TOOL_DIR)
    warnings = compare_snapshot(excel_data, snapshot)
    if warnings:
        for w in warnings:
            print(w)
    else:
        _ok("필드 번호 변경 없음")

    # ------------------------------------------------------------------
    # Step 5: .proto 파일 생성 + 스냅샷 저장
    # ------------------------------------------------------------------
    _step(5, TOTAL_STEPS, ".proto 파일 생성")
    proto_path = generate_proto(excel_data, config, TOOL_DIR)
    save_snapshot(excel_data, TOOL_DIR)
    _ok(f"생성: {proto_path}")

    # ------------------------------------------------------------------
    # Step 6: protoc 실행
    # ------------------------------------------------------------------
    _step(6, TOTAL_STEPS, "protoc 실행 (Python 코드 생성)")
    from cs_generator import run_protoc

    run_protoc(config, TOOL_DIR)
    py_out_dir = TOOL_DIR / "output" / "py"
    py_files = list(py_out_dir.glob("*_pb2.py"))
    _ok(f"Python pb2 파일: {[f.name for f in py_files]}")

    # ------------------------------------------------------------------
    # Step 7: 데이터 직렬화 → .bytes
    # ------------------------------------------------------------------
    _step(7, TOTAL_STEPS, "데이터 직렬화 → .bytes")
    from data_serializer import serialize_all

    bytes_files = serialize_all(excel_data, config, TOOL_DIR)
    for bf in bytes_files:
        _ok(f"직렬화: {Path(bf).name}")

    # ------------------------------------------------------------------
    # Step 8: .bytes → UE5 DataTable JSON 변환
    # ------------------------------------------------------------------
    _step(8, TOTAL_STEPS, ".bytes → UE5 DataTable JSON 변환")
    if args.skip_json:
        print("  --skip-json 옵션으로 인해 JSON 변환을 건너뜁니다.")
        json_files = []
    else:
        from json_exporter import export_bytes_to_json

        bytes_dir = TOOL_DIR / "output" / "bytes"
        pb2_dir = TOOL_DIR / "output" / "py"
        json_dir = TOOL_DIR / "output" / "json"
        table_names = list(excel_data.data.keys())

        json_files = export_bytes_to_json(
            bytes_dir, pb2_dir, json_dir,
            config["package_name"], table_names,
        )

    # ------------------------------------------------------------------
    # 요약 리포트
    # ------------------------------------------------------------------
    print("\n" + "=" * 60)
    print("  실행 완료 요약")
    print("=" * 60)
    print(f"  처리한 데이터 시트 수  : {len(excel_data.data)}")
    print(f"  정의된 메시지 수       : {len(excel_data.schema)}")
    print(f"  정의된 Enum 수         : {len(excel_data.enums)}")
    print(f"  생성된 .bytes 파일 수  : {len(bytes_files)}")
    print(f"  생성된 JSON 파일 수    : {len(json_files)}")
    print(f"  생성된 .proto 경로     : {proto_path}")
    if warnings:
        print(f"\n  ⚠ 필드 번호 변경 경고: {len(warnings)}건")
    print("=" * 60)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> None:
    args = _parse_args()

    tool_dir_str = str(TOOL_DIR)
    if tool_dir_str not in sys.path:
        sys.path.insert(0, tool_dir_str)

    try:
        run_pipeline(args)
    except KeyboardInterrupt:
        print("\n중단되었습니다.", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"\n오류 발생: {e}", file=sys.stderr)
        if "--debug" in sys.argv:
            traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
