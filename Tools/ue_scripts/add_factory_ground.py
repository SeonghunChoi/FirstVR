"""8-3b: 팩 퍼시스턴트 레벨(바닥 지형·블로킹 볼륨·폴리지)을 Showcase/Map/L_Combat_Factory_Floor로 '다른 이름으로 저장'한다.

EditorAssetLibrary.duplicate_asset으로 맵을 복사하면 레벨이 제대로 복제되지 않는다(다시 열 때 지형 트랜스폼이 기본값으로
돌아가고 블로킹 볼륨 브러시가 빠졌다). 그래서 원본 맵을 열고 메모리에서 데모용 요소를 지운 뒤 save_map으로 새 이름에 저장한다.
원본 파일은 저장하지 않는다. 지우는 것: 조명 시나리오·환경 스트리밍 레벨 목록, PlayerStart, 월드 세팅 게임 모드,
레벨 블루프린트(데모 메뉴·페이드 위젯을 띄우고 조명 시나리오를 스트리밍했다).
끝나면 L_Combat_Factory를 다시 연다(이 레벨을 항상 로드 스트리밍 레벨로 이미 갖고 있다).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

SRC = "/Game/AbandonedFactory/Demo/Maps/Warehouse_02_old_street/Street_01_P"
DST = "/Game/Showcase/Map/L_Combat_Factory_Floor"
OLD = "L_Combat_Factory_Ground"   # duplicate_asset으로 만들었던 망가진 복제본(스트리밍 목록에서 뺀다)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

if unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages():
    raise RuntimeError("Current map has unsaved changes")
LES.load_level(SRC)
if EAL.does_asset_exist(DST):
    raise RuntimeError(f"{DST} already exists")

world = UES.get_editor_world()
for lvl in unreal.EditorLevelUtils.get_levels(world):
    if lvl.get_outer().get_name() != "Street_01_P":
        unreal.EditorLevelUtils.remove_level_from_world(lvl)
for a in EAS.get_all_level_actors():
    if a.get_class().get_name() == "PlayerStart":
        EAS.destroy_actor(a)
world.get_world_settings().set_editor_property("default_game_mode", None)
lsb = unreal.find_object(None, f"{SRC}.Street_01_P:PersistentLevel.Street_01_P")
for n in G.get_nodes(BEL.find_event_graph(lsb)):
    G.remove_node(n)
BEL.remove_unused_variables(lsb)
BEL.compile_blueprint(lsb)

print("save_map", unreal.EditorLoadingAndSavingUtils.save_map(world, DST))
print("now", UES.get_editor_world().get_path_name())
LES.load_level("/Game/Showcase/Map/L_Combat_Factory")
world = UES.get_editor_world()
for lvl in unreal.EditorLevelUtils.get_levels(world):
    if lvl.get_outer().get_name() == OLD:
        unreal.EditorLevelUtils.remove_level_from_world(lvl)
if "L_Combat_Factory_Floor" not in [l.get_outer().get_name() for l in unreal.EditorLevelUtils.get_levels(world)]:
    unreal.EditorLevelUtils.add_level_to_world(world, DST, unreal.LevelStreamingAlwaysLoaded)
LES.set_current_level_by_name("L_Combat_Factory")
LES.save_current_level()
print("levels", [l.get_outer().get_name() for l in unreal.EditorLevelUtils.get_levels(world)])
ls = [a for a in EAS.get_all_level_actors() if a.get_class().get_name() == "Landscape"]
print("landscape", [(a.get_actor_location(), a.get_actor_scale3d()) for a in ls])
