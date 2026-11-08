"""8-5c: L_Combat_Factory의 게임 모드 오버라이드를 공장 전용 BP_FirstVRFactoryMode로 바꾸고 맵을 저장한다."""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

MAP = "/Game/Showcase/Map/L_Combat_Factory"
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

world = UES.get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    dirty = unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()
    if dirty:
        raise RuntimeError(f"unsaved maps: {[str(p) for p in dirty]}")
    LES.load_level(MAP)
    world = UES.get_editor_world()
ws = world.get_world_settings()
print("before", ws.get_editor_property("default_game_mode"))
ws.modify()
ws.set_editor_property("default_game_mode", bp_class(load("/Game/Showcase/Game/BP_FirstVRFactoryMode")))
print("after", ws.get_editor_property("default_game_mode"))
LES.save_current_level()
print("saved", world.get_path_name())
