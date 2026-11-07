"""8-2d 결정: RU74만 조정간을 AK 순서(안전 → 연발 → 단발)로. 물려받은 BP_TriggerInput의 FireModes를 자식에서 덮어쓴다."""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

bp = load("/Game/Showcase/Weapon/BP_AutoRifleWeapon_RU74")
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary
template = None
for h in sds.k2_gather_subobject_data_for_blueprint(bp):
    data = lib.get_data(h)
    if str(lib.get_variable_name(data)) == "BP_TriggerInput":
        print("inherited", lib.is_inherited_component(data))
        template = lib.get_object_for_blueprint(data, bp)  # 물려받은 컴포넌트는 자식 BP용 덮어쓰기 템플릿
if template is None:
    raise RuntimeError("BP_TriggerInput not found")
print("template", template.get_path_name())
print("before", template.get_editor_property("FireModes"))
template.modify()
template.set_editor_property("FireModes", [unreal.FireMode.AUTO, unreal.FireMode.SINGLE])
print("after", template.get_editor_property("FireModes"))
compile_save(bp)
