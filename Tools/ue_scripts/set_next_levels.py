"""8-6e: 레벨 잇기 — 게임 모드의 NextLevel(승리 화면 '다음 레벨' 버튼)을 정한다.

    python Tools/ue_remote.py Tools/ue_scripts/set_next_levels.py   # L_Lab이 열려 있어야 함

- Showcase/Game/BP_FirstVRFactoryMode(공장): NextLevel = L_Lab, 글자 "Next Level"
- Showcase/Game/BP_FirstVRLabMode(새로 만듦, Portfolio 구간 게임 모드 BP_FirstVRSectionGameMode의 자식): NextLevel = L_Combat_Factory,
  글자 "Start Over". Portfolio(git) 에셋은 Showcase 맵을 참조하지 않게 레벨 연결은 Showcase 자식에만 둔다.
- L_Lab 월드 세팅 게임 모드 → BP_FirstVRLabMode
NextLevel은 소프트 참조라 경로 글자로 넣는다(맵을 불러오지 않음).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

FACTORY_MODE = "/Game/Showcase/Game/BP_FirstVRFactoryMode"
LAB_MODE = "/Game/Showcase/Game/BP_FirstVRLabMode"
SECTION_MODE = "/Game/Portfolio/Game/BP_FirstVRSectionGameMode"
L_LAB = "/Game/Showcase/Map/L_Lab.L_Lab"
L_FACTORY = "/Game/Showcase/Map/L_Combat_Factory.L_Combat_Factory"


def set_next(bp, level, label):
    cdo = unreal.get_default_object(bp_class(bp))
    set_path(cdo, "NextLevel", level)   # 실패하면 예외
    cdo.set_editor_property("next_level_label", label)
    compile_save(bp)
    print(bp.get_name(), "->", G.get_property_by_path(cdo, "NextLevel"), str(cdo.get_editor_property("next_level_label")))


set_next(load(FACTORY_MODE), L_LAB, "Next Level")

if EAL.does_asset_exist(LAB_MODE):
    lab = load(LAB_MODE)
else:
    folder, name = LAB_MODE.rsplit("/", 1)
    f = unreal.BlueprintFactory()
    f.set_editor_property("parent_class", bp_class(load(SECTION_MODE)))
    lab = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.Blueprint, f)
set_next(lab, L_FACTORY, "Start Over")

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_path_name() != L_LAB:
    raise RuntimeError("open L_Lab to set its game mode")
world.get_world_settings().set_editor_property("default_game_mode", bp_class(lab))
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print("L_Lab game mode", world.get_world_settings().get_editor_property("default_game_mode").get_name())
