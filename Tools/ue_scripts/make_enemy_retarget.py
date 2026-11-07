"""8-5a: UE5 마네킹 → UE4 뼈대(Wasteland Warrior) 리타깃 준비.

Showcase/Enemy/Retarget/에 IK 리그 둘(자동 리타깃 정의)과 IK 리타기터(기본 작업 + 체인 자동 매핑 + 뼈 자동 정렬)를 만든다.
두 캐릭터(Wasteland Warrior, Sci-Fi Soldier 02)는 같은 UE4 뼈대 구조라 대상은 Warrior 하나로 만든다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

FOLDER = "/Game/Showcase/Enemy/Retarget"
SOURCE_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
TARGET_MESH = "/Game/WastelandWarrior/meshes/W_warrior_ver1_SK"
SRC_T = unreal.RetargetSourceOrTarget

for name in ("IK_UE5Manny", "IK_UE4Warrior", "RTG_UE5Manny_UE4Warrior"):
    if EAL.does_asset_exist(f"{FOLDER}/{name}"):
        raise RuntimeError(f"{FOLDER}/{name} already exists")


def make_rig(name, mesh_path):
    rig = unreal.IKRigDefinitionFactory.create_new_ik_rig_asset(FOLDER, name)
    c = unreal.IKRigController.get_controller(rig)
    c.set_skeletal_mesh(load(mesh_path))
    ok = c.apply_auto_generated_retarget_definition()
    chains = [str(ch.get_editor_property("chain_name")) for ch in c.get_retarget_chains()]
    print(name, "auto retarget definition", ok, "root", c.get_retarget_root(), "chains", len(chains), chains)
    EAL.save_loaded_asset(rig)
    return rig


src_rig = make_rig("IK_UE5Manny", SOURCE_MESH)
tgt_rig = make_rig("IK_UE4Warrior", TARGET_MESH)

rtg = unreal.AssetToolsHelpers.get_asset_tools().create_asset("RTG_UE5Manny_UE4Warrior", FOLDER, unreal.IKRetargeter, unreal.IKRetargetFactory())
rc = unreal.IKRetargeterController.get_controller(rtg)
rc.set_ik_rig(SRC_T.SOURCE, src_rig)
rc.set_ik_rig(SRC_T.TARGET, tgt_rig)
rc.set_preview_mesh(SRC_T.SOURCE, load(SOURCE_MESH))
rc.set_preview_mesh(SRC_T.TARGET, load(TARGET_MESH))
rc.remove_all_ops()
rc.add_default_ops()
rc.assign_ik_rig_to_all_ops(SRC_T.SOURCE, src_rig)
rc.assign_ik_rig_to_all_ops(SRC_T.TARGET, tgt_rig)
rc.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
rc.auto_align_all_bones(SRC_T.TARGET)
print("ops", [rc.get_op_name(i) for i in range(rc.get_num_retarget_ops())])
EAL.save_loaded_asset(rtg)
print("saved", rtg.get_path_name())
