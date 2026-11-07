"""후보 자리마다 바닥 높이를 찾아 눈높이(바닥+170) 카메라를 놓는다. 한 번 실행에 한 장면.

    python Tools/ue_remote.py Tools/ue_scripts/eye_shots.py <x> <y> <yaw>
"""
import sys
import unreal

x, y, yaw = map(float, sys.argv[1:4])
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
ign = []
floor = None
for _ in range(10):
    r = unreal.SystemLibrary.line_trace_single(w, unreal.Vector(x, y, 3000), unreal.Vector(x, y, -2000),
                                               unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, ign, unreal.DrawDebugTrace.NONE, True)
    if r is None:
        break
    h = r.to_tuple()
    if not h[0]:
        break
    name = h[9].get_actor_label() if h[9] else ""
    floor = h[4].z
    if "Landscape" in name or "Ground" in name or h[4].z < 400:
        print("floor", round(floor, 1), name)
        break
    ign.append(h[9])
eye = unreal.Vector(x, y, (floor or 0) + 170)
unreal.AutomationLibrary.set_editor_active_viewport_view_mode(unreal.ViewModeIndex.VMI_LIT)
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(eye, unreal.Rotator(0, -5, yaw))
print("eye", eye, "yaw", yaw)
