"""
cs_generator.py
---------------
protoc 바이너리를 subprocess로 실행하여 C# 및 Python 코드를 생성한다.

출력:
  output/cs/  : C# 소스 파일 (.cs)
  output/py/  : Python protobuf 모듈 (_pb2.py) — data_serializer.py에서 사용
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def run_protoc(config: dict, tool_dir: Path) -> None:
    """
    protoc 바이너리를 실행하여 .proto 파일로부터 C# 및 Python 코드를 생성한다.

    Args:
        config:   config.yaml에서 로드한 설정 dict
        tool_dir: Tool 디렉터리 경로

    Raises:
        FileNotFoundError: protoc 바이너리를 찾을 수 없을 때
        RuntimeError:      protoc 실행 실패 시 (stderr 포함)
    """
    protoc_path = tool_dir / config["protoc_path"]

    # Windows에서 .exe 확장자 자동 추가
    if sys.platform == "win32" and not str(protoc_path).lower().endswith(".exe"):
        protoc_path = Path(str(protoc_path) + ".exe")

    if not protoc_path.exists():
        raise FileNotFoundError(
            f"protoc 바이너리를 찾을 수 없습니다: {protoc_path}\n"
            "다운로드: https://github.com/protocolbuffers/protobuf/releases\n"
            f"다운로드 후 '{tool_dir / 'tools'}' 폴더에 배치하세요.\n"
            "  Windows : tools/protoc.exe\n"
            "  Linux   : tools/protoc\n"
            "  macOS   : tools/protoc"
        )

    package_name = config["package_name"]
    proto_dir  = tool_dir / "output" / "proto"
    cs_out     = tool_dir / "output" / "cs"
    py_out     = tool_dir / "output" / "py"
    proto_file = proto_dir / f"{package_name}.proto"

    # 출력 디렉터리 생성
    cs_out.mkdir(parents=True, exist_ok=True)
    py_out.mkdir(parents=True, exist_ok=True)

    if not proto_file.exists():
        raise FileNotFoundError(
            f".proto 파일을 찾을 수 없습니다: {proto_file}\n"
            "generate_proto()를 먼저 실행하세요."
        )

    cmd = [
        str(protoc_path),
        f"--csharp_out={cs_out}",
        f"--python_out={py_out}",
        f"-I={proto_dir}",
        str(proto_file),
    ]

    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
    )

    if result.returncode != 0:
        raise RuntimeError(
            f"protoc 실행 실패 (exit code {result.returncode})\n"
            f"명령어: {' '.join(cmd)}\n"
            f"--- stderr ---\n{result.stderr}\n"
            f"--- stdout ---\n{result.stdout}"
        )

    if result.stderr.strip():
        # 경고 메시지 출력 (오류가 아니어도 stderr에 출력될 수 있음)
        print(f"  protoc 경고:\n{result.stderr.strip()}")
