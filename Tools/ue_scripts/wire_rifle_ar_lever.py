"""8-2d: BP_RifleWeapon_AR이 안전장치 상태를 ABP_Rifle_AR에 넘긴다.

RifleMesh 애님 클래스 = ABP_Rifle_AR
함수 UpdateSafetyLever: RifleMesh -> GetAnimInstance -> Cast To ABP_Rifle_AR -> Set bSafetyLocked(BP_SafetyStep.IsLocked)
BeginPlay(Hide Bone 뒤)와 BP_SafetyStep의 OnSafetyLockChanged에서 부른다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

bp = load("/Game/Showcase/Weapon/BP_RifleWeapon_AR")
abp = load("/Game/Showcase/Weapon/ABP_Rifle_AR")
abp_class = bp_class(abp)
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
step = G.add_variable_node(fg, "BP_SafetyStep", False, None, 450, 250)
is_locked = G.add_call_function_node(fg, load_cls("/Script/FirstVR.SafetyStep"), "IsLocked", 650, 250)
set_locked = G.add_variable_node(fg, "bSafetyLocked", True, abp_class, 850, 0)
link(mesh, "RifleMesh", get_anim, "self")
link(get_anim, "ReturnValue", cast, "Object")
link(entry, "then", cast, "execute")
link(step, "BP_SafetyStep", is_locked, "self")
link(cast, "then", set_locked, "execute")
cast_out = [p[4:].split(" (")[0] for p in G.describe_pins(cast) if p.startswith("out") and "object/" in p][0]
link(cast, cast_out, set_locked, "self")
link(is_locked, "ReturnValue", set_locked, "bSafetyLocked")
BEL.compile_blueprint(bp)

# --- 부르는 곳: BeginPlay(Hide Bone 뒤), 안전장치 바뀔 때
eg = graph(bp)
hide = find_node(eg, "Hide Bone by Name")
call_begin = G.add_call_function_node(eg, bp_class(bp), FUNC, 750, -48)
link(hide, "then", call_begin, "execute")
changed = G.add_component_event_node(eg, "BP_SafetyStep", "OnSafetyLockChanged", 0, 650)
call_changed = G.add_call_function_node(eg, bp_class(bp), FUNC, 400, 650)
link(changed, "then", call_changed, "execute")

dump_graph(fg, pins=False)
dump_graph(eg, pins=False)
compile_save(bp)
