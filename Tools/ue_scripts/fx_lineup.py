"""9-2e: 이펙트 후보를 시뮬레이트 월드 바닥에 줄지어 재생하고(회전 0 = 이펙트의 +Z가 위) 잠깐 뒤 찍는다. 반복 재생하는 이펙트인지도 본다.

    python Tools/ue_remote.py Tools/ue_scripts/fx_lineup.py <이름> <x> <y> <z> <간격> <에셋 경로>...      # 재생하고 찍기
    python Tools/ue_remote.py Tools/ue_scripts/fx_lineup.py check                                   # 남아 있는(반복) 이펙트 보고
카메라는 줄의 앞쪽 비스듬히 위. 결과는 스크래치가 아니라 Saved/Captures/<이름>.png
"""
import sys
import unreal

U = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
w = U.get_game_world()
TAG = "FxLineup"


def main():
    if sys.argv[1] == "check":
        for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Emitter):
            pass
        comps = [c for c in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor)]
        alive = []
        for c in unreal.ObjectIterator if False else []:
            pass
        print("use log"); return
    name = sys.argv[1]
    unreal.SystemLibrary.execute_console_command(w, "slomo 0.1")   # 짧은 피격 효과가 찍힐 때까지 남게
    x, y, z, gap = map(float, sys.argv[2:6])
    paths = sys.argv[6:]
    for i, p in enumerate(paths):
        fx = unreal.load_asset(p)
        loc = unreal.Vector(x + i * gap, y, z)
        if isinstance(fx, unreal.NiagaraSystem):
            c = unreal.NiagaraFunctionLibrary.spawn_system_at_location(w, fx, loc)
        else:
            c = unreal.GameplayStatics.spawn_emitter_at_location(w, fx, loc)
        print(i, p.split("/")[-1], loc, c.get_name() if c else None)
    mid = unreal.Vector(x + gap * (len(paths) - 1) / 2, y, z)
    eye = mid + unreal.Vector(0, -gap * len(paths) * 0.55, gap * 0.9)
    U.set_level_viewport_camera_info(eye, unreal.MathLibrary.find_look_at_rotation(eye, mid + unreal.Vector(0, 0, 30)))
    unreal.AutomationLibrary.take_high_res_screenshot(1600, 900, f"D:/Projects/Unreal/FirstVR/Saved/Captures/{name}.png",
                                                      delay=float(0.01), force_game_view=True)


main()
