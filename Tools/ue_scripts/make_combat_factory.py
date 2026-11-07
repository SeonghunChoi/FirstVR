"""8-3a: 꾸민 전투 레벨 뼈대 L_Combat_Factory.

엔진 기본 레벨 템플릿(Template_Default: 해·하늘빛·스카이 애트머스피어·안개·구름)으로 새 레벨을 만들고,
팩 맵의 환경 서브레벨만 항상 로드되는 스트리밍 레벨로 붙인다. 팩의 조명 시나리오(UE4 방식, UE5에서 검게 나옴)는 쓰지 않는다.
템플릿의 바닥 메시는 지운다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

PATH = "/Game/Showcase/Map/L_Combat_Factory"
ENV = "/Game/AbandonedFactory/Demo/Maps/Warehouse_02_old_street/Sublevels/Street_01_Environment"

if unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages():
    raise RuntimeError("Current map has unsaved changes")
if EAL.does_asset_exist(PATH):
    raise RuntimeError(f"{PATH} already exists")

LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not LES.new_level_from_template(PATH, "/Engine/Maps/Templates/Template_Default"):
    raise RuntimeError("new_level_from_template failed")

for a in EAS.get_all_level_actors():
    if isinstance(a, unreal.StaticMeshActor) and "Floor" in a.get_actor_label():
        print("remove template floor", a.get_actor_label())
        EAS.destroy_actor(a)

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
streaming = unreal.EditorLevelUtils.add_level_to_world(world, ENV, unreal.LevelStreamingAlwaysLoaded)
print("streaming", streaming.get_editor_property("world_asset") if streaming else None)

for a in EAS.get_all_level_actors():
    if a.get_level().get_outer().get_name() == "L_Combat_Factory":
        print("  ", a.get_class().get_name(), a.get_actor_label())
LES.save_current_level()
print("saved", PATH)
