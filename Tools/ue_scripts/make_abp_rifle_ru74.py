"""8-2d: 연발 소총 레버 애님 BP ABP_Rifle_RU74.

무기 BP가 bSafetyLocked·bAutoMode를 넘기고, 이 애님 BP가 세 위치(안전·연발·단발) 각도로 b_SafetyLever를 돌린다.
AK 계열 레버는 위에서부터 안전 → 연발 → 단발 순서다. 회전축은 본 로컬 X(메시 옆 방향)라서 Make Rotator의 Roll을 쓴다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

FOLDER = "/Game/Showcase/Weapon"
NAME = "ABP_Rifle_RU74"
PATH = f"{FOLDER}/{NAME}"
BONE = "b_SafetyLever"
ANGLES = {"SafeAngle": -20.0, "AutoAngle": -7.5, "SingleAngle": 0.0}  # 사용자가 SK_RU74에서 b_SafetyLever를 돌려 잼(레퍼런스 포즈 = 단발)

KML = load_cls("/Script/Engine.KismetMathLibrary")
ANIM_INSTANCE = unreal.AnimInstance.static_class()

if EAL.does_asset_exist(PATH):  # 이 스크립트가 만든 것만 다시 만든다
    unreal.SystemLibrary.collect_garbage()
    EAL.delete_asset(PATH)

skeleton = load("/Game/FPS_Assault_Pack/Meshes/RU74/SK_RU74_Skeleton")
factory = unreal.AnimBlueprintFactory()
factory.set_editor_property("target_skeleton", skeleton)
factory.set_editor_property("parent_class", ANIM_INSTANCE)
abp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, FOLDER, unreal.AnimBlueprint, factory)

BOOL = BEL.get_basic_type_by_name("bool")
REAL = BEL.get_basic_type_by_name("real")
for name, ty in [("bSafetyLocked", BOOL), ("bAutoMode", BOOL), ("SafeAngle", REAL), ("AutoAngle", REAL),
                 ("SingleAngle", REAL), ("LeverSpeed", REAL), ("LeverAngle", REAL), ("bLeverReady", BOOL)]:
    BEL.add_member_variable(abp, name, ty)

# --- 이벤트 그래프: 목표 각도 = 잠김이면 SafeAngle, 아니면 연발이면 AutoAngle, 단발이면 SingleAngle
eg = graph(abp)
update = G.add_event_node(eg, ANIM_INSTANCE, "BlueprintUpdateAnimation", 0, 0)
auto = G.add_variable_node(eg, "bAutoMode", False, None, -300, 300)
auto_angle = G.add_variable_node(eg, "AutoAngle", False, None, -300, 360)
single_angle = G.add_variable_node(eg, "SingleAngle", False, None, -300, 420)
mode_select = G.add_call_function_node(eg, KML, "SelectFloat", -50, 320)
link(auto_angle, "AutoAngle", mode_select, "A")
link(single_angle, "SingleAngle", mode_select, "B")
link(auto, "bAutoMode", mode_select, "bPickA")

locked = G.add_variable_node(eg, "bSafetyLocked", False, None, 0, 200)
safe = G.add_variable_node(eg, "SafeAngle", False, None, 0, 260)
select = G.add_call_function_node(eg, KML, "SelectFloat", 250, 220)
link(safe, "SafeAngle", select, "A")
link(mode_select, "ReturnValue", select, "B")
link(locked, "bSafetyLocked", select, "bPickA")

ready = G.add_variable_node(eg, "bLeverReady", False, None, 250, 120)
branch = G.add_node(eg, unreal.load_class(None, "/Script/BlueprintGraph.K2Node_IfThenElse"), 300, 0)
link(update, "then", branch, "execute")
link(ready, "bLeverReady", branch, "Condition")

snap = G.add_variable_node(eg, "LeverAngle", True, None, 600, 150)
link(branch, "else", snap, "execute")
link(select, "ReturnValue", snap, "LeverAngle")
mark = G.add_variable_node(eg, "bLeverReady", True, None, 850, 150)
link(snap, "then", mark, "execute")
set_default(mark, "bLeverReady", "true")

current = G.add_variable_node(eg, "LeverAngle", False, None, 400, -200)
speed = G.add_variable_node(eg, "LeverSpeed", False, None, 400, -140)
interp = G.add_call_function_node(eg, KML, "FInterpTo_Constant", 600, -200)
link(current, "LeverAngle", interp, "Current")
link(select, "ReturnValue", interp, "Target")
link(update, "DeltaTimeX", interp, "DeltaTime")
link(speed, "LeverSpeed", interp, "InterpSpeed")
follow = G.add_variable_node(eg, "LeverAngle", True, None, 900, -100)
link(branch, "then", follow, "execute")
link(interp, "ReturnValue", follow, "LeverAngle")

# --- 애님 그래프: Mesh Space Ref Pose -> Transform (Modify) Bone -> Component To Local -> Output Pose
ag = graph(abp, "AnimGraph")
root = find_node(ag, "AnimGraphNode_Root")
ref = G.add_node(ag, load_cls("/Script/AnimGraph.AnimGraphNode_MeshRefPose"), -700, 0)
modify = G.add_node(ag, load_cls("/Script/AnimGraph.AnimGraphNode_ModifyBone"), -450, 0)
to_local = G.add_node(ag, load_cls("/Script/AnimGraph.AnimGraphNode_ComponentToLocalSpace"), -150, 0)
set_path(modify, "Node.BoneToModify.BoneName", BONE)
set_path(modify, "Node.RotationMode", "BMM_Additive")
set_path(modify, "Node.RotationSpace", "BCS_BoneSpace")
link(ref, "ComponentPose", modify, "ComponentPose")
link(modify, "Pose", to_local, "ComponentPose")
link(to_local, "Pose", root, "Result")
angle = G.add_variable_node(ag, "LeverAngle", False, None, -900, 200)
rot = G.add_call_function_node(ag, KML, "MakeRotator", -700, 200)
link(angle, "LeverAngle", rot, "Roll")
link(rot, "ReturnValue", modify, "Rotation")

BEL.compile_blueprint(abp)
cdo = unreal.get_default_object(BEL.generated_class(abp))
cdo.set_editor_property("bSafetyLocked", True)
for k, v in ANGLES.items():
    cdo.set_editor_property(k, v)
cdo.set_editor_property("LeverSpeed", 900.0)
compile_save(abp)
