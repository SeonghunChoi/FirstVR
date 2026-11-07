"""실행 중인 월드(없으면 에디터 월드)에 콘솔 명령을 보낸다.
    python Tools/ue_remote.py Tools/ue_scripts/console.py "Enemy.Kill" [반복 횟수]
"""
import sys
import unreal
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
w = UES.get_game_world() or UES.get_editor_world()
n = int(sys.argv[2]) if len(sys.argv) > 2 else 1
for _ in range(n):
    unreal.SystemLibrary.execute_console_command(w, sys.argv[1])
print("sent", sys.argv[1], "x", n, "to", w.get_name())
