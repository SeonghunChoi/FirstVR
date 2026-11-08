"""8-5b: UE4 뼈대용 적 애님 BP ABP_Enemy_UE4 (ABP_EnemyMannequin과 같은 그래프).

부모는 C++ UEnemyAnimInstance(GroundSpeed·PelvisOffset·Left/RightFootOffset·LegIKAlpha를 계산)라 이벤트 그래프는 비어 있다.
애님 그래프:
  블렌드 스페이스(BS_Idle_Walk_Run_UE4, Speed ← GroundSpeed) → Slot DefaultSlot → Local To Component
  → Transform (Modify) Bone ×3(pelvis, ik_foot_l, ik_foot_r, Add to Existing, Component Space) → Leg IK(Alpha ← LegIKAlpha)
  → Component To Local → Output Pose
UE4 마네킹 뼈대에도 pelvis·ik_foot_l/r·foot_l/r가 같은 이름으로 있다.
8-5d: 이 스크립트로 다시 만들었다면 이어서 add_stride_warping.py(발 미끄러짐 보폭 워핑)를 실행한다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

FOLDER = "/Game/Showcase/Enemy"
NAME = "ABP_Enemy_UE4"
PATH = f"{FOLDER}/{NAME}"
SKELETON = "/Game/WastelandWarrior/meshes/UE4_Mannequin_Skeleton"
BLEND_SPACE = "/Game/Showcase/Enemy/Anims/BS_Idle_Walk_Run_UE4"
ANIM = "/Script/AnimGraph."

if EAL.does_asset_exist(PATH):  # 이 스크립트가 만든 것만 다시 만든다
    unreal.SystemLibrary.collect_garbage()
    EAL.delete_asset(PATH)

parent = load_cls("/Script/FirstVR.EnemyAnimInstance")
factory = unreal.AnimBlueprintFactory()
factory.set_editor_property("target_skeleton", load(SKELETON))
factory.set_editor_property("parent_class", parent)
abp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, FOLDER, unreal.AnimBlueprint, factory)

ag = graph(abp, "AnimGraph")
root = find_node(ag, "AnimGraphNode_Root")

bs = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_BlendSpacePlayer"), -1232, 48)
bs_asset = load(BLEND_SPACE)
set_path(bs, "Node.BlendSpace", f"/Script/Engine.BlendSpace'{bs_asset.get_path_name()}'")
speed = G.add_variable_node(ag, "GroundSpeed", False, None, -1450, 120)
link(speed, "GroundSpeed", bs, "Y")

slot = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_Slot"), -976, 176)
set_path(slot, "Node.SlotName", "DefaultSlot")
link(bs, "Pose", slot, "Source")

to_comp = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_LocalToComponentSpace"), -768, 320)
link(slot, "Pose", to_comp, "LocalPose")

prev, prev_pin = to_comp, "ComponentPose"
for i, (bone, var) in enumerate([("pelvis", "PelvisOffset"), ("ik_foot_l", "LeftFootOffset"), ("ik_foot_r", "RightFootOffset")]):
    y = -256 + i * 384
    m = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_ModifyBone"), -256, y)
    set_path(m, "Node.BoneToModify.BoneName", bone)
    set_path(m, "Node.TranslationMode", "BMM_Additive")
    set_path(m, "Node.TranslationSpace", "BCS_ComponentSpace")
    v = G.add_variable_node(ag, var, False, None, -512, y + 80)
    link(v, var, m, "Translation")
    link(prev, prev_pin, m, "ComponentPose")
    prev, prev_pin = m, "Pose"

leg = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_LegIK"), 128, 112)
set_path(leg, "Node.LegsDefinition",
         '((IKFootBone=(BoneName="ik_foot_l"),FKFootBone=(BoneName="foot_l")),'
         '(IKFootBone=(BoneName="ik_foot_r"),FKFootBone=(BoneName="foot_r")))')
alpha = G.add_variable_node(ag, "LegIKAlpha", False, None, 128, 32)
link(alpha, "LegIKAlpha", leg, "Alpha")
link(prev, prev_pin, leg, "ComponentPose")

to_local = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_ComponentToLocalSpace"), 400, 320)
link(leg, "Pose", to_local, "ComponentPose")
link(to_local, "Pose", root, "Result")

dump_graph(ag, pins=False)
compile_save(abp)
for key in ("BlendSpace", "Slot", "다리 IK", "Leg IK"):
    n = find_node(ag, key)
    if n:
        print(key, "->", G.get_property_by_path(n, "Node")[:200])
