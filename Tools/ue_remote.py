"""열려 있는 언리얼 에디터에 Python을 보내 실행한다 (Python Editor Script Plugin의 원격 실행).

사용법:
    python Tools/ue_remote.py <스크립트.py> [인수...]   # 파일 실행 (에디터 안에서 sys.argv로 인수를 받는다)
    python Tools/ue_remote.py -c "<파이썬 코드>"         # 코드 문자열 실행

에디터에서 `프로젝트 세팅 > Python > Enable Remote Execution`이 켜져 있어야 한다 (DefaultEngine.ini에 설정됨).
에디터 쪽 출력(print, unreal.log)과 오류를 그대로 보여 주고, 실패하면 종료 코드 1.
"""

import os
import sys
import time

sys.stdout.reconfigure(encoding="utf-8")
sys.stderr.reconfigure(encoding="utf-8")

ENGINE_PYTHON = r"F:\Epic Games\Games\UE_5.7\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python"
sys.path.insert(0, ENGINE_PYTHON)

import remote_execution  # noqa: E402


def find_node(remote, timeout):
    deadline = time.time() + timeout
    while time.time() < deadline:
        nodes = remote.remote_nodes
        if nodes:
            return nodes[0]
        time.sleep(0.1)
    return None


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2

    if sys.argv[1] == "-c":
        command = sys.argv[2]
    else:
        path = os.path.abspath(sys.argv[1])
        args = " ".join(f'"{a}"' for a in sys.argv[2:])
        command = f'"{path}" {args}'.strip()

    remote = remote_execution.RemoteExecution()
    remote.start()
    try:
        node = find_node(remote, timeout=5.0)
        if not node:
            print("No editor found. Is the editor open with remote execution enabled?", file=sys.stderr)
            return 1
        remote.open_command_connection(node)
        result = remote.run_command(command, unattended=True, exec_mode=remote_execution.MODE_EXEC_FILE)
    finally:
        remote.stop()

    for entry in result.get("output", []):
        text = entry.get("output", "").rstrip("\n")
        stream = sys.stderr if entry.get("type") in ("Error", "Warning") else sys.stdout
        print(text, file=stream)
    if result.get("result") not in (None, "None", ""):
        print(result["result"])
    return 0 if result.get("success") else 1


if __name__ == "__main__":
    sys.exit(main())
