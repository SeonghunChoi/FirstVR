"""리타깃 비교 사진용: 원본(UE5 마네킹)과 리타깃(UE4 캐릭터)을 나란히 같은 시각의 자세로 세운다(임시 액터, 맵은 변경 안 됨).

    python Tools/ue_remote.py Tools/ue_scripts/pose_compare.py <원본 애니메이션> <리타깃 애니메이션> <시각(초)> [clear]
clear를 주면 이 스크립트가 만든 액터를 지우기만 한다.
"""
import sys
import unreal

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
TAG = "PoseCompare"
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
for a in unreal.GameplayStatics.get_all_actors_with_tag(world, TAG):   # 임시(transient) 액터는 EditorActorSubsystem 목록에 안 나온다
    a.destroy_actor()
if len(sys.argv) > 1 and sys.argv[1] == "clear":
    raise SystemExit

src_anim, dst_anim, t = sys.argv[1], sys.argv[2], float(sys.argv[3])
BASE = unreal.Vector(0, 3000, 0)   # 레벨 원점에서 떨어진 빈 곳
for i, (mesh, anim) in enumerate([("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple", src_anim),
                                  ("/Game/WastelandWarrior/meshes/W_warrior_ver1_SK", dst_anim)]):
    a = EAS.spawn_actor_from_class(unreal.SkeletalMeshActor, BASE + unreal.Vector(0, i * 150, 0), unreal.Rotator(0, 0, 0), transient=True)
    a.tags = [TAG]
    c = a.skeletal_mesh_component
    c.set_skinned_asset_and_update(unreal.load_asset(mesh))
    c.set_update_animation_in_editor(True)    # 에디터 월드에서는 이걸 켜야 애니메이션이 평가된다
    c.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    c.set_animation(unreal.load_asset(anim))
    c.set_play_rate(0.0)
    c.set_position(t, False)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
eye = BASE + unreal.Vector(420, 75, 120)
UES.set_level_viewport_camera_info(eye, unreal.MathLibrary.find_look_at_rotation(eye, BASE + unreal.Vector(0, 75, 90)))
unreal.AutomationLibrary.set_editor_active_viewport_view_mode(unreal.ViewModeIndex.VMI_LIT)
print("posed", src_anim.split('/')[-1], "vs", dst_anim.split('/')[-1], "at", t)
