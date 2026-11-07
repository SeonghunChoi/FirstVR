"""8-3 성능: 팩 환경 서브레벨 Street_01_Environment를 Showcase/Map/L_Combat_Factory_Env로 '다른 이름으로 저장'하고
L_Combat_Factory의 스트리밍 레벨을 그 복사본으로 바꾼다(이후 지우기는 복사본에만 한다). 원본은 저장하지 않는다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

SRC = "/Game/AbandonedFactory/Demo/Maps/Warehouse_02_old_street/Sublevels/Street_01_Environment"
DST = "/Game/Showcase/Map/L_Combat_Factory_Env"
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

if unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages():
    raise RuntimeError("Current map has unsaved changes")
if EAL.does_asset_exist(DST):
    raise RuntimeError(f"{DST} already exists")
LES.load_level(SRC)
world = UES.get_editor_world()
gm = world.get_world_settings().get_editor_property("default_game_mode")
print("source game mode", gm)
world.get_world_settings().set_editor_property("default_game_mode", None)
lsb = unreal.find_object(None, f"{SRC}.Street_01_Environment:PersistentLevel.Street_01_Environment")
nodes = G.get_nodes(BEL.find_event_graph(lsb)) if lsb else []
print("level script nodes", len(nodes))
for n in nodes:
    G.remove_node(n)
if lsb:
    BEL.remove_unused_variables(lsb)
    BEL.compile_blueprint(lsb)
print("save_map", unreal.EditorLoadingAndSavingUtils.save_map(world, DST))

LES.load_level("/Game/Showcase/Map/L_Combat_Factory")
world = UES.get_editor_world()
for lvl in unreal.EditorLevelUtils.get_levels(world):
    if lvl.get_outer().get_name() == "Street_01_Environment":
        unreal.EditorLevelUtils.remove_level_from_world(lvl)
unreal.EditorLevelUtils.add_level_to_world(world, DST, unreal.LevelStreamingAlwaysLoaded)
LES.set_current_level_by_name("L_Combat_Factory")
LES.save_current_level()
print("levels", [l.get_outer().get_name() for l in unreal.EditorLevelUtils.get_levels(world)])
