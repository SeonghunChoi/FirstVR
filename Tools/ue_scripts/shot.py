"""9-2b: 포트폴리오 스크린샷. 에디터 뷰포트 카메라를 맞추고 고해상도로 찍는다(에디터 아이콘·기즈모 없이 게임 화면처럼).

    python Tools/ue_remote.py Tools/ue_scripts/shot.py <이름> look <ex> <ey> <ez> <tx> <ty> <tz> [size=<가로>x<세로>]
    python Tools/ue_remote.py Tools/ue_scripts/shot.py <이름> rot <ex> <ey> <ez> <pitch> <yaw> [size=...]
    python Tools/ue_remote.py Tools/ue_scripts/shot.py <이름> here [size=...]      # 지금 뷰포트 카메라 그대로
    python Tools/ue_remote.py Tools/ue_scripts/shot.py - look ...                    # 카메라만 맞추고 찍지 않음(구도 보기는 window_shot.ps1)

결과는 Docs/Portfolio/Shots/<이름>.png (git 제외 폴더). 찍기는 다음 틱에 끝나므로 확인은 다음 실행에서.
시뮬레이트·PIE 중에도 에디터 뷰포트를 찍는다(시뮬레이트 월드가 보이는 그 뷰포트).
"""
import os
import sys
import unreal

OUT = r"D:\Projects\Unreal\FirstVR\Docs\Portfolio\Shots"
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

args = sys.argv[1:]
opts = dict(a.split("=", 1) for a in args if "=" in a)
args = [a for a in args if "=" not in a]
name, mode = args[0], args[1]
w, h = map(int, opts.get("size", "2560x1440").split("x"))

if mode == "look":
    e = unreal.Vector(*map(float, args[2:5])); t = unreal.Vector(*map(float, args[5:8]))
    UES.set_level_viewport_camera_info(e, unreal.MathLibrary.find_look_at_rotation(e, t))
elif mode == "rot":   # rot <ex> <ey> <ez> <pitch> <yaw>  (바로 아래를 볼 때: pitch -89.9)
    e = unreal.Vector(*map(float, args[2:5]))
    UES.set_level_viewport_camera_info(e, unreal.Rotator(0, float(args[5]), float(args[6])))  # roll, pitch, yaw
unreal.AutomationLibrary.set_editor_active_viewport_view_mode(unreal.ViewModeIndex.VMI_LIT)
loc, rot = UES.get_level_viewport_camera_info()
print("camera", loc, rot)

if name != "-":
    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, name + ".png")
    unreal.AutomationLibrary.take_high_res_screenshot(w, h, path, delay=float(opts.get("delay", "2")), force_game_view=True)
    print("shot ->", path, f"{w}x{h}")
    # 그림에 이름표를 얹을 때 월드 위치 → 화면 위치를 계산할 수 있게 카메라를 남긴다(에디터 뷰포트 수평 FOV 90)
    with open(os.path.join(OUT, "shots_log.txt"), "a", encoding="utf-8") as f:
        f.write(f"{name}\t{loc.x:.1f},{loc.y:.1f},{loc.z:.1f}\t{rot.pitch:.3f},{rot.yaw:.3f},{rot.roll:.3f}\t{w}x{h}\n")
