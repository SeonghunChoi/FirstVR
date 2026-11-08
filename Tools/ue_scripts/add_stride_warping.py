"""8-5d 발 미끄러짐: 적 애님 BP에 Stride Warping을 끼워 넣는다(make_abp_enemy_ue4.py·ABP_EnemyMannequin과 같은 그래프 구조).

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/add_stride_warping.py [애님 BP] [블렌드 스페이스]
기본은 ABP_Enemy_UE4. 블렌드 스페이스를 주면 블렌드 스페이스 플레이어의 에셋을 그것으로 바꾼다(걷기 샘플을 옮긴 사본).

  블렌드 스페이스 → [로컬→컴포넌트] → Stride Warping(Graph 모드, LocomotionSpeed ← GroundSpeed: 보폭 = 실제 속도 / 애니메이션 루트 모션 속도)
  → Leg IK(항상, 줄인 ik_foot에 다리를 맞춤) → [컴포넌트→로컬] → Slot DefaultSlot → (기존) 절뚝임 보정 → Leg IK(LegIKAlpha) → …
보폭 워핑은 슬롯 앞이라 공격 몽타주에는 영향이 없다. 이미 들어가 있으면 멈춘다.
애니메이션의 ik_foot_root·ik_foot_l/r이 UE5 원본처럼 root·발을 따라가야 한다. 리타깃한 애니메이션은 먼저 fix_retarget_ik_bones.py로 고친다
(안 고치면 Leg IK가 발을 몸 뒤의 목표로 당겨 두 다리가 접힌다. 애님 그래프의 Copy Bone으로 맞추는 방법은 런타임에 반영되지 않았다).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

ABP = sys.argv[1] if len(sys.argv) > 1 else "/Game/Showcase/Enemy/ABP_Enemy_UE4"
BLEND_SPACE = sys.argv[2] if len(sys.argv) > 2 else None
ANIM = "/Script/AnimGraph."
WARP = "/Script/AnimationWarpingEditor."

abp = load(ABP)
ag = graph(abp, "AnimGraph")
if find_node(ag, "AnimGraphNode_StrideWarping"):
    raise RuntimeError("Stride Warping already added")

bs = find_node(ag, "AnimGraphNode_BlendSpacePlayer")
slot = find_node(ag, "AnimGraphNode_Slot")
to_comp = find_node(ag, "AnimGraphNode_LocalToComponentSpace")
pelvis = find_node(ag, "Bone: pelvis")
for n in (bs, slot, to_comp, pelvis):
    if not n:
        raise RuntimeError("graph is not the one make_abp_enemy_ue4.py builds")

def pos_x(n):
    """노드 x 위치(DescribeNode의 @(x, y)에서 읽음)."""
    return int(G.describe_node(n).rsplit("@(", 1)[1].split(",")[0])


# 기존 노드를 오른쪽으로 비켜 자리를 만든다
for n in G.get_nodes(ag):
    if n not in (bs,) and "GroundSpeed" not in G.describe_node(n):
        set_path(n, "NodePosX", pos_x(n) + 1800)

G.break_pin_links(bs, "Pose")

if BLEND_SPACE:
    set_path(bs, "Node.BlendSpace", f"/Script/Engine.BlendSpace'{load(BLEND_SPACE).get_path_name()}'")

# --- 슬롯 앞: 보폭 워핑
a = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_LocalToComponentSpace"), -960, 48)
link(bs, "Pose", a, "LocalPose")
prev, prev_pin = a, "ComponentPose"

warp = G.add_node(ag, load_cls(WARP + "AnimGraphNode_StrideWarping"), -160, 48)
set_path(warp, "Node.Mode", "Graph")
set_path(warp, "Node.PelvisBone.BoneName", "pelvis")
set_path(warp, "Node.IKFootRootBone.BoneName", "ik_foot_root")
set_path(warp, "Node.FootDefinitions",
         '((IKFootBone=(BoneName="ik_foot_l"),FKFootBone=(BoneName="foot_l"),ThighBone=(BoneName="thigh_l")),'
         '(IKFootBone=(BoneName="ik_foot_r"),FKFootBone=(BoneName="foot_r"),ThighBone=(BoneName="thigh_r")))')
speed = G.add_variable_node(ag, "GroundSpeed", False, None, -400, 200)
link(speed, "GroundSpeed", warp, "LocomotionSpeed")
link(prev, prev_pin, warp, "ComponentPose")

leg = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_LegIK"), 160, 48)
set_path(leg, "Node.LegsDefinition",
         '((IKFootBone=(BoneName="ik_foot_l"),FKFootBone=(BoneName="foot_l")),'
         '(IKFootBone=(BoneName="ik_foot_r"),FKFootBone=(BoneName="foot_r")))')
link(warp, "Pose", leg, "ComponentPose")

b = G.add_node(ag, load_cls(ANIM + "AnimGraphNode_ComponentToLocalSpace"), 420, 48)
link(leg, "Pose", b, "ComponentPose")
link(b, "Pose", slot, "Source")

dump_graph(ag, pins=False)
compile_save(abp)
print("warp", G.get_property_by_path(warp, "Node")[:400])
