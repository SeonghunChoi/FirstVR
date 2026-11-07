"""8-2d: BP_AutoRifleWeapon_RU74가 안전장치·발사 모드를 ABP_Rifle_RU74에 넘긴다.

RifleMesh 애님 클래스 = ABP_Rifle_RU74
함수 UpdateSafetyLever: RifleMesh -> GetAnimInstance -> Cast -> Set bSafetyLocked(BP_SafetyStep.IsLocked)
                        -> Set bAutoMode(BP_TriggerInput.GetFireMode == Auto)
BeginPlay(Hide Bone 뒤), OnSafetyLockChanged(BP_SafetyStep), OnFireModeChanged(BP_TriggerInput)에서 부른다.
조정간은 안전 → 단발로 갈 때 발사 모드를 먼저 바꾸고 잠금을 푼다. 함수가 매번 두 값을 현재 상태에서 다시 읽으므로 순서와 상관없다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

bp = load("/Game/Showcase/Weapon/BP_AutoRifleWeapon_RU74")
abp_class = bp_class(load("/Game/Showcase/Weapon/ABP_Rifle_RU74"))
FUNC = "UpdateSafetyLever"

# --- RifleMesh 애님 클래스
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary
mesh_template = None
for h in sds.k2_gather_subobject_data_for_blueprint(bp):
    data = lib.get_data(h)
    if str(lib.get_variable_name(data)) == "RifleMesh":
        mesh_template = lib.get_object(data)  # 템플릿이 필요해서 옛 API를 쓴다
if mesh_template is None:
    raise RuntimeError("RifleMesh not found")
mesh_template.set_editor_property("animation_mode", unreal.AnimationMode.ANIMATION_BLUEPRINT)
mesh_template.set_editor_property("anim_class", abp_class)

# --- 함수 그래프
if BEL.find_graph(bp, FUNC):
    raise RuntimeError(f"{FUNC} already exists (reload the package to retry)")
fg = BEL.add_function_graph(bp, FUNC)
entry = find_node(fg, "K2Node_FunctionEntry")
mesh = G.add_variable_node(fg, "RifleMesh", False, None, 0, 150)
get_anim = G.add_call_function_node(fg, unreal.SkeletalMeshComponent.static_class(), "GetAnimInstance", 200, 150)
cast = G.add_cast_node(fg, abp_class, 450, 0)
link(mesh, "RifleMesh", get_anim, "self")
link(get_anim, "ReturnValue", cast, "Object")
link(entry, "then", cast, "execute")
cast_out = [p[4:].split(" (")[0] for p in G.describe_pins(cast) if p.startswith("out") and "object/" in p][0]

step = G.add_variable_node(fg, "BP_SafetyStep", False, None, 450, 250)
is_locked = G.add_call_function_node(fg, load_cls("/Script/FirstVR.SafetyStep"), "IsLocked", 650, 250)
link(step, "BP_SafetyStep", is_locked, "self")
set_locked = G.add_variable_node(fg, "bSafetyLocked", True, abp_class, 850, 0)
link(cast, "then", set_locked, "execute")
link(cast, cast_out, set_locked, "self")
link(is_locked, "ReturnValue", set_locked, "bSafetyLocked")

trigger = G.add_variable_node(fg, "BP_TriggerInput", False, None, 650, 400)
get_mode = G.add_call_function_node(fg, load_cls("/Script/FirstVR.TriggerInputComponent"), "GetFireMode", 850, 400)
link(trigger, "BP_TriggerInput", get_mode, "self")
is_auto = G.add_node(fg, unreal.load_class(None, "/Script/BlueprintGraph.K2Node_EnumEquality"), 1050, 400)
link(get_mode, "ReturnValue", is_auto, "A")
set_default(is_auto, "B", "Auto")
set_auto = G.add_variable_node(fg, "bAutoMode", True, abp_class, 1250, 0)
link(set_locked, "then", set_auto, "execute")
link(cast, cast_out, set_auto, "self")
link(is_auto, "ReturnValue", set_auto, "bAutoMode")
BEL.compile_blueprint(bp)

# --- 부르는 곳
eg = graph(bp)
hide = find_node(eg, "Hide Bone by Name")
call_begin = G.add_call_function_node(eg, bp_class(bp), FUNC, 750, -32)
link(hide, "then", call_begin, "execute")
for i, (comp, delegate) in enumerate([("BP_SafetyStep", "OnSafetyLockChanged"), ("BP_TriggerInput", "OnFireModeChanged")]):
    ev = G.add_component_event_node(eg, comp, delegate, 0, 650 + i * 200)
    call = G.add_call_function_node(eg, bp_class(bp), FUNC, 400, 650 + i * 200)
    link(ev, "then", call, "execute")

for p in G.describe_pins(is_auto):
    print("enum==", p)
dump_graph(fg, pins=False)
dump_graph(eg, pins=False)
compile_save(bp)
