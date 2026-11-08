"""8-5d 조사: 실행 중인 월드에서 걷는 적의 다리 본 위치(액터 기준 cm)를 출력한다. 변경 없음(뼈 갱신 옵션만 켬).

    python Tools/ue_remote.py Tools/ue_scripts/leg_pose.py [샘플 수]
액터 원점은 캡슐 중심이라 바닥은 약 z -88. 무릎(calf)이 발보다 많이 낮거나 발이 바닥에서 높으면 다리가 접힌 것이다.
"""
import sys
import unreal

UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
w = UES.get_game_world()
if not w:
    raise RuntimeError("not playing")
BONES = ["root", "pelvis", "thigh_l", "calf_l", "foot_l", "ik_foot_l", "thigh_r", "calf_r", "foot_r", "ik_foot_r", "ik_foot_root"]
for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Character):
    if "Enemy" not in a.get_class().get_name():
        continue
    m = a.get_editor_property("mesh")
    m.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
    t = a.get_actor_transform()
    v = a.get_velocity()
    parts = []
    for b in BONES:
        p = t.inverse_transform_location(m.get_socket_location(b))
        parts.append(f"{b} ({p.x:.0f},{p.y:.0f},{p.z:.0f})")
    print(a.get_name(), f"speed {(v.x ** 2 + v.y ** 2) ** 0.5:.0f}", " ".join(parts))
