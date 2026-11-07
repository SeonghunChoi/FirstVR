"""8-5b 확인용: UE5 마네킹 + ABP_EnemyMannequin과 UE4 캐릭터 + ABP_Enemy_UE4를 나란히 세운다(임시 액터, 맵은 변경 안 됨).

    python Tools/ue_remote.py Tools/ue_scripts/abp_compare.py idle
    python Tools/ue_remote.py Tools/ue_scripts/abp_compare.py montage <번호 1~3> <시각(초)>
    python Tools/ue_remote.py Tools/ue_scripts/abp_compare.py clear
idle은 애님 BP가 블렌드 스페이스(속도 0 = 대기)를 돌리는 모습, montage는 같은 공격 몽타주를 슬롯으로 재생해 그 시각에 멈춘 모습.
"""
import sys
import unreal

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
TAG = "AbpCompare"
world = UES.get_editor_world()
for a in unreal.GameplayStatics.get_all_actors_with_tag(world, TAG):  # 임시 액터는 EditorActorSubsystem 목록에 안 나온다
    a.destroy_actor()
mode = sys.argv[1] if len(sys.argv) > 1 else "idle"
if mode == "clear":
    raise SystemExit

BASE = unreal.Vector(0, 3000, 0)
PAIRS = [("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple", "/Game/Portfolio/Enemies/ABP_EnemyMannequin",
          "/Game/Portfolio/Enemies/Attack/AM_Attack_0{n}"),
         ("/Game/WastelandWarrior/meshes/W_warrior_ver1_SK", "/Game/Showcase/Enemy/ABP_Enemy_UE4",
          "/Game/Showcase/Enemy/Anims/AM_Attack_0{n}_UE4")]
for i, (mesh, abp, montage) in enumerate(PAIRS):
    a = EAS.spawn_actor_from_class(unreal.SkeletalMeshActor, BASE + unreal.Vector(0, i * 150, 0), unreal.Rotator(0, 0, 0), transient=True)
    a.tags = [TAG]
    c = a.skeletal_mesh_component
    c.set_skinned_asset_and_update(unreal.load_asset(mesh))
    c.set_update_animation_in_editor(True)  # 에디터 월드에서는 이걸 켜야 애니메이션이 평가된다
    c.set_animation_mode(unreal.AnimationMode.ANIMATION_BLUEPRINT)
    c.set_anim_instance_class(unreal.load_asset(abp).generated_class())
    inst = c.get_anim_instance()
    print(i, mesh.split("/")[-1], "anim instance", inst.get_class().get_name() if inst else None)
    if mode == "montage" and inst:
        n, t = sys.argv[2], float(sys.argv[3])
        m = unreal.load_asset(montage.format(n=n))
        length = inst.montage_play(m, 1.0, unreal.MontagePlayReturnType.MONTAGE_LENGTH, t, True)
        inst.montage_pause(m)
        print("   montage", m.get_name(), "length", round(length, 2), "paused at", t)

eye = BASE + unreal.Vector(420, 75, 120)
UES.set_level_viewport_camera_info(eye, unreal.MathLibrary.find_look_at_rotation(eye, BASE + unreal.Vector(0, 75, 90)))
unreal.AutomationLibrary.set_editor_active_viewport_view_mode(unreal.ViewModeIndex.VMI_LIT)
