"""에디터 뷰포트 카메라·보기 모드를 맞춘다. 찍기는 Tools/window_shot.ps1.

    python Tools/ue_remote.py Tools/ue_scripts/view.py top <x> <y> <height> [lit|unlit]
    python Tools/ue_remote.py Tools/ue_scripts/view.py look <ex> <ey> <ez> <tx> <ty> <tz> [lit|unlit]
"""
import sys
import unreal

UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
args = sys.argv[1:]
mode = args[-1] if args and args[-1] in ("lit", "unlit") else "lit"
if mode in args:
    args = args[:-1]
unreal.AutomationLibrary.set_editor_active_viewport_view_mode(
    unreal.ViewModeIndex.VMI_UNLIT if mode == "unlit" else unreal.ViewModeIndex.VMI_LIT)

if args[0] == "top":
    x, y, h = map(float, args[1:4])
    UES.set_level_viewport_camera_info(unreal.Vector(x, y, h), unreal.Rotator(0, -90, 0))  # roll, pitch, yaw 순서
elif args[0] == "look":
    e = unreal.Vector(*map(float, args[1:4])); t = unreal.Vector(*map(float, args[4:7]))
    UES.set_level_viewport_camera_info(e, unreal.MathLibrary.find_look_at_rotation(e, t))
loc, rot = UES.get_level_viewport_camera_info()
print("camera", loc, rot)
