"""8-5e 확인용: Wasteland Warrior와 Sci-Fi Soldier(Wasteland 뼈대에 붙인 사본)를 같은 애니메이션으로 나란히 세운다(임시 액터, 맵 변경 없음).

    python Tools/ue_remote.py Tools/ue_scripts/compare_scifi.py idle               # ABP_Enemy_UE4 대기
    python Tools/ue_remote.py Tools/ue_scripts/compare_scifi.py montage <1~3> <초>  # 공격 몽타주를 그 시각에 멈춤
    python Tools/ue_remote.py Tools/ue_scripts/compare_scifi.py walk <초>           # 걷기 시퀀스를 그 시각에 멈춤
    python Tools/ue_remote.py Tools/ue_scripts/compare_scifi.py feet               # 세운 뒤 한 틱 지나서 발·골반·머리 높이
    python Tools/ue_remote.py Tools/ue_scripts/compare_scifi.py clear
찍기는 Tools/window_shot.ps1.
"""
import sys
import unreal

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
TAG = "ScifiCompare"
world = UES.get_editor_world()
mode = sys.argv[1] if len(sys.argv) > 1 else "idle"
if mode == "feet":   # 앞서 세운 두 캐릭터의 발 본·발끝 높이(바닥 0 기준)
    for a in unreal.GameplayStatics.get_all_actors_with_tag(world, TAG):
        c = a.skeletal_mesh_component
        print(c.get_skinned_asset().get_name(), {b: round(c.get_socket_location(b).z, 1) for b in ("foot_l", "foot_r", "ball_l", "ball_r", "pelvis", "head")})
    raise SystemExit
for a in unreal.GameplayStatics.get_all_actors_with_tag(world, TAG):
    a.destroy_actor()
if mode == "clear":
    raise SystemExit

ABP = "/Game/Showcase/Enemy/ABP_Enemy_UE4"
WALK = "/Game/Showcase/Enemy/Anims/MF_Unarmed_Walk_Fwd_UE4"
MESHES = ["/Game/WastelandWarrior/meshes/W_warrior_ver1_SK", "/Game/Showcase/Enemy/Meshes/SK_SciFiSoldier02_Enemy"]
BASE = unreal.Vector(-500, -500, 0)   # L_Combat 방 안 바닥(z 0)

for i, mesh in enumerate(MESHES):
    a = EAS.spawn_actor_from_class(unreal.SkeletalMeshActor, BASE + unreal.Vector(0, i * 120, 0), unreal.Rotator(0, 0, 0), transient=True)   # Rotator 인자 순서는 (roll, pitch, yaw). 메시는 +Y를 보고 카메라는 옆(+X)에서 본다
    a.tags = [TAG]
    c = a.skeletal_mesh_component
    c.set_skinned_asset_and_update(unreal.load_asset(mesh))
    c.set_update_animation_in_editor(True)
    if mode == "walk":
        c.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
        c.play_animation(unreal.load_asset(WALK), True)
        c.set_position(float(sys.argv[2]), False)
        c.set_play_rate(0.0)
    else:
        c.set_animation_mode(unreal.AnimationMode.ANIMATION_BLUEPRINT)
        c.set_anim_instance_class(unreal.load_asset(ABP).generated_class())
        inst = c.get_anim_instance()
        if mode == "montage" and inst:
            m = unreal.load_asset(f"/Game/Showcase/Enemy/Anims/AM_Attack_0{sys.argv[2]}_UE4")
            inst.montage_play(m, 1.0, unreal.MontagePlayReturnType.MONTAGE_LENGTH, float(sys.argv[3]), True)
            inst.montage_pause(m)
    print(i, mesh.split("/")[-1], "spawned")

eye = BASE + unreal.Vector(420, 60, 95)
UES.set_level_viewport_camera_info(eye, unreal.MathLibrary.find_look_at_rotation(eye, BASE + unreal.Vector(0, 60, 80)))
unreal.AutomationLibrary.set_editor_active_viewport_view_mode(unreal.ViewModeIndex.VMI_LIT)
