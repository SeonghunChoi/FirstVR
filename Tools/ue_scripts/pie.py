"""시뮬레이트(PIE Simulate) 시작·종료와 실행 중 월드 찾기.

    python Tools/ue_remote.py Tools/ue_scripts/pie.py start [/Game/맵 경로]
    python Tools/ue_remote.py Tools/ue_scripts/pie.py stop
VR Preview는 시뮬레이터 조작이 필요해서 사용자가 한다. 여기서는 VR 없이 로직과 애니메이션 값만 본다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)


def game_world():
    return UES.get_game_world()


def find_actor(class_name_part):
    w = game_world()
    for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor):
        if class_name_part in a.get_class().get_name():
            return a
    return None


if __name__ == "__main__" and len(sys.argv) > 1:
    cmd = sys.argv[1]
    if cmd == "start":
        if len(sys.argv) > 2:
            if unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages():
                raise RuntimeError("Current map has unsaved changes")
            LES.load_level(sys.argv[2])
        LES.editor_play_simulate()
        print("simulate requested")
    elif cmd == "stop":
        LES.editor_request_end_play()
        print("end play requested")
