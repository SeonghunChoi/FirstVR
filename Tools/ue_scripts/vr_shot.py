"""9-2d: VR Preview 중인 게임 화면(관전자 화면)을 원격으로 찍는다. 사용자가 시뮬레이터로 자세를 잡고 있을 때 쓴다.

    python Tools/ue_remote.py Tools/ue_scripts/vr_shot.py <이름> [pre=<콘솔 명령>;<콘솔 명령>...] [size=<가로>x<세로>|<배수>]

pre의 명령(예: Player.Damage 20, slomo 0.2)을 먼저 보내고 같은 틱에 HighResShot을 건다. 피격 비네트처럼 짧은 효과도 잡힌다.
결과는 Docs/Portfolio/Shots/vr/<이름>.png (git 제외 폴더). 찍기는 다음 프레임에 끝나므로 확인은 다음 실행에서.
"""
import os
import sys
import unreal

OUT = r"D:/Projects/Unreal/FirstVR/Docs/Portfolio/Shots/vr"


def main():
    args = sys.argv[1:]
    opts = dict(a.split("=", 1) for a in args if "=" in a and not a.startswith("pre="))
    pre = [a[4:] for a in args if a.startswith("pre=")]
    name = [a for a in args if "=" not in a][0]
    w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    if not w:
        raise RuntimeError("VR Preview(PIE)가 켜져 있지 않다")
    for group in pre:
        for cmd in group.split(";"):
            if cmd.strip():
                unreal.SystemLibrary.execute_console_command(w, cmd.strip())
                print("sent", cmd.strip())
    os.makedirs(OUT, exist_ok=True)
    size = opts.get("size", "1")   # VR 한쪽 눈 3360x1760이 1배
    unreal.SystemLibrary.execute_console_command(w, f"HighResShot {size} filename={OUT}/{name}.png")
    print("shot ->", f"{OUT}/{name}.png", "size", size, "world", w.get_name())


main()
