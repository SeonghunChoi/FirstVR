"""8-3b: 팩 퍼시스턴트 레벨(바닥 지형·블로킹 볼륨·폴리지·PlayerStart)을 Showcase로 복제해 PlayerStart와 월드 세팅 게임 모드를 지우고
L_Combat_Factory에 항상 로드 스트리밍 레벨로 붙인다. 팩 원본은 고치지 않는다."""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

SRC = "/Game/AbandonedFactory/Demo/Maps/Warehouse_02_old_street/Street_01_P"
DST = "/Game/Showcase/Map/L_Combat_Factory_Ground"
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

if not EAL.does_asset_exist(DST):
    if not EAL.duplicate_asset(SRC, DST):
        raise RuntimeError("duplicate failed")
    print("duplicated", DST)
world = UES.get_editor_world()
if "L_Combat_Factory_Ground" not in [l.get_outer().get_name() for l in unreal.EditorLevelUtils.get_levels(world)]:
    unreal.EditorLevelUtils.add_level_to_world(world, DST, unreal.LevelStreamingAlwaysLoaded)
LES.set_current_level_by_name("L_Combat_Factory")
for a in EAS.get_all_level_actors():
    if a.get_level().get_outer().get_name() == "L_Combat_Factory_Ground":
        print("  ", a.get_class().get_name(), a.get_actor_label())
        if a.get_class().get_name() == "PlayerStart":
            EAS.destroy_actor(a); print("   -> removed PlayerStart")
# 팩 데모 게임 모드 참조를 지운다(데모 캐릭터와 컴파일 오류가 있는 ThirdPerson_AnimBP를 끌고 와 Play 때 대화 상자가 뜬다)
lvl = [l for l in unreal.EditorLevelUtils.get_levels(world) if l.get_outer().get_name() == "L_Combat_Factory_Ground"][0]
for ws in unreal.ObjectIterator(unreal.WorldSettings):
    if ws.get_outer() == lvl:
        ws.modify(); ws.set_editor_property("default_game_mode", None)
# 복제된 레벨 블루프린트도 비운다(팩 데모 메뉴·페이드 위젯을 화면에 띄우고, BeginPlay에서 팩 조명 시나리오를 스트리밍했다)
lsb = unreal.find_object(None, f"{DST}.L_Combat_Factory_Ground:PersistentLevel.L_Combat_Factory_Ground")
for n in G.get_nodes(BEL.find_event_graph(lsb)):
    G.remove_node(n)
BEL.remove_unused_variables(lsb)
BEL.compile_blueprint(lsb)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, False)
print("dirty", [p.get_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()])
