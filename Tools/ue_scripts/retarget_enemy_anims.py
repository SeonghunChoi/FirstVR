"""8-5a: 적 애니메이션 하나를 UE4 뼈대로 리타깃해 Showcase/Enemy/Anims/로 옮긴다(한 번에 하나씩).

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/retarget_enemy_anims.py <원본 에셋 경로> [refs]

여러 개(블렌드 스페이스 + 몽타주 + 참조 시퀀스)를 한 번에 돌리면 공격 시퀀스 압축에서 에디터가 멈췄다(2026-11-08).
그래서 에셋 하나씩, 기본은 참조 에셋을 따라가지 않는다(refs를 주면 따라간다).
IKRetargetBatchOperation은 결과를 원본 옆이나 /Game 루트에 만들므로 이름 끝에 _UE4를 붙여 만든 뒤 옮긴다.
이미 옮겨 둔 결과가 있으면 다시 만들지 않는다.
8-5d: 리타기터는 IK 본(ik_foot_root·ik_foot_l/r)을 옮기지 않는다. 리타깃한 뒤 fix_retarget_ik_bones.py를 실행한다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

OUT = "/Game/Showcase/Enemy/Anims"
src = sys.argv[1]
follow = len(sys.argv) > 2 and sys.argv[2] == "refs"
name = src.split("/")[-1]
if EAL.does_asset_exist(f"{OUT}/{name}_UE4"):
    print("already done", f"{OUT}/{name}_UE4")
    raise SystemExit

ar = unreal.AssetRegistryHelpers.get_asset_registry()
data = ar.get_asset_by_object_path(f"{src}.{name}")  # noqa (5.7에서 deprecated, 동작은 함)
made = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
    [data], load("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"), load("/Game/WastelandWarrior/meshes/W_warrior_ver1_SK"),
    load("/Game/Showcase/Enemy/Retarget/RTG_UE5Manny_UE4Warrior"), "", "", "", "_UE4", follow, False)
moved = []
for d in made:
    p = str(d.package_name)
    dst = f"{OUT}/{p.split('/')[-1]}"
    if p != dst:
        if EAL.does_asset_exist(dst):
            EAL.delete_asset(dst)     # 따로 먼저 만든 것이 있으면 새로 만든 것(이 묶음이 참조하는 것)으로 바꾼다
            print("replaced", dst)
        if not EAL.rename_asset(p, dst):
            raise RuntimeError(f"move failed {p}")
    moved.append(dst)
# 다 옮긴 뒤에 저장한다. 옮기는 도중에 저장하면 먼저 저장한 블렌드 스페이스·몽타주 파일에 옮기기 전 시퀀스 경로가 남는다(8-5c에서 발견).
for dst in moved:
    EAL.save_asset(dst, only_if_is_dirty=False)
    print("retargeted", dst)
