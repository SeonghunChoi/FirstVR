"""8-6b: 구간 진행 시험 배치 — 구간 게임 모드·구간 BP·시험용 웨이브 데이터를 만들고 L_Lab에 구간 4개를 놓는다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/make_lab_sections.py assets   # BP·데이터 에셋
    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/make_lab_sections.py level    # L_Lab 배치(열려 있어야 함)

assets
- Portfolio/Game/BP_FirstVRSectionGameMode: C++ AFirstVRSectionGameMode 자식, 폰·실패/승리 화면은 BP_FirstVRGameMode와 같게.
- Portfolio/Game/BP_EncounterSection: C++ AEncounterSection 자식, 표식 = 엔진 원기둥을 납작하게(지름 1.5m).
- Showcase/Game/DA_Waves_Lab_S1~S3: 시험용 웨이브(시작 2초 뒤 Sci-Fi Soldier 1명, 스폰 지점 Lab_S1~S3). 8-6d에서 실제 구성으로 바꾼다.
level (방 A 기준 좌표, 바닥 z 0)
- 스폰 지점 Lab_S1·S2(첫 방), Lab_S3(둘째 방), 폴더 Gameplay.
- 구간 1 시작 자리(PlayerStart 자리) / 2 첫 방 입구 복도 앞(문: 두 방 사이 게이트 2개) / 3 둘째 방(문: 탈출문) / 4 탈출 복도(웨이브 없음).
- 월드 세팅 게임 모드 → BP_FirstVRSectionGameMode.
이미 있으면 멈춘다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

tools = unreal.AssetToolsHelpers.get_asset_tools()
MODE = "/Game/Portfolio/Game/BP_FirstVRSectionGameMode"
SECTION = "/Game/Portfolio/Game/BP_EncounterSection"
WAVES = "/Game/Showcase/Game/DA_Waves_Lab_S{n}"
SOLDIER = "/Game/Showcase/Enemy/BP_EnemySciFiSoldier"
cmd = sys.argv[1] if len(sys.argv) > 1 else "assets"


def child_bp(path, parent_cls):
    if EAS_exists(path):
        raise RuntimeError(f"{path} already exists")
    folder, name = path.rsplit("/", 1)
    f = unreal.BlueprintFactory()
    f.set_editor_property("parent_class", parent_cls)
    return tools.create_asset(name, folder, unreal.Blueprint, f)


def EAS_exists(path):
    return EAL.does_asset_exist(path)


if cmd == "assets":
    # 게임 모드
    mode = child_bp(MODE, load_cls("/Script/FirstVR.FirstVRSectionGameMode"))
    src = unreal.get_default_object(bp_class(load("/Game/Portfolio/Game/BP_FirstVRGameMode")))
    cdo = unreal.get_default_object(bp_class(mode))
    for k in ("default_pawn_class", "fail_screen_class", "victory_screen_class", "result_screen_distance"):
        cdo.set_editor_property(k, src.get_editor_property(k))
    compile_save(mode)

    # 구간 BP(표식 메시)
    sec = child_bp(SECTION, load_cls("/Script/FirstVR.EncounterSection"))
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary
    for h in sds.k2_gather_subobject_data_for_blueprint(sec):
        data = lib.get_data(h)
        if str(lib.get_variable_name(data)) == "Marker":
            m = lib.get_object_for_blueprint(data, sec)
            m.modify()
            m.set_editor_property("static_mesh", load("/Engine/BasicShapes/Cylinder"))
            m.set_editor_property("relative_scale3d", unreal.Vector(1.5, 1.5, 0.02))
            m.set_editor_property("relative_location", unreal.Vector(0, 0, 1))
    compile_save(sec)

    # 시험용 웨이브 데이터
    for n in (1, 2, 3):
        path = WAVES.format(n=n)
        if EAL.does_asset_exist(path):
            raise RuntimeError(f"{path} already exists")
        folder, name = path.rsplit("/", 1)
        f = unreal.DataAssetFactory()
        f.set_editor_property("data_asset_class", load_cls("/Script/FirstVR.WaveDataAsset"))
        wd = tools.create_asset(name, folder, None, f)
        group = unreal.WaveSpawnGroup()
        group.set_editor_property("enemy_class", bp_class(load(SOLDIER)))
        group.set_editor_property("count", 1)
        group.set_editor_property("spawn_point_name", f"Lab_S{n}")
        wave = unreal.WaveDefinition()
        wave.set_editor_property("groups", [group])
        wd.set_editor_property("start_delay", 2.0)
        wd.set_editor_property("waves", [wave])
        EAL.save_loaded_asset(wd, only_if_is_dirty=False)
        print("waves", path, wd.get_editor_property("waves"))

elif cmd == "level":
    UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    world = UES.get_editor_world()
    if world.get_path_name().split(".")[0] != "/Game/Showcase/Map/L_Lab":
        raise RuntimeError("open L_Lab first")
    actors = EAS.get_all_level_actors()
    if any(a.get_class().get_name().startswith("BP_EncounterSection") for a in actors):
        raise RuntimeError("sections already placed")
    gates = {a.get_actor_label(): a for a in actors if "Gate" in a.get_class().get_name()}
    print("gates", list(gates))
    link_gates = [gates["BP_MGate01-500x350-1"], gates["BP_MGate01-500x350-2_B"]]
    exit_gate = [gates["BP_MGate01-500x350-2_Exit"]]

    spawn_cls = load_cls("/Script/FirstVR.EnemySpawnPoint")
    for name, loc, yaw in (("Lab_S1", (-500, -400, 0), 30), ("Lab_S2", (-300, 1250, 0), -30), ("Lab_S3", (4000, 1250, 0), 180)):
        sp = EAS.spawn_actor_from_class(spawn_cls, unreal.Vector(*loc), unreal.Rotator(0, 0, yaw))
        sp.set_editor_property("spawn_point_name", name)
        sp.set_actor_label(f"SpawnPoint_{name}")
        sp.set_folder_path("Gameplay")

    sec_cls = bp_class(load(SECTION))
    specs = [
        (1, (300, 400, 0), WAVES.format(n=1), []),
        (2, (900, 400, 0), WAVES.format(n=2), link_gates),
        (3, (3300, 400, 0), WAVES.format(n=3), exit_gate),
        (4, (4425, -1000, 0), None, []),
    ]
    for order, loc, waves, doors in specs:
        s = EAS.spawn_actor_from_class(sec_cls, unreal.Vector(*loc))
        s.set_editor_property("section_order", order)
        if waves:
            s.set_editor_property("wave_data", load(waves))
        s.set_editor_property("doors_to_unlock", doors)
        s.set_actor_label(f"Section_{order}" + ("_Escape" if not waves else ""))
        s.set_folder_path("Gameplay")
        print("section", order, loc, waves, [d.get_actor_label() for d in doors])

    world.get_world_settings().set_editor_property("default_game_mode", bp_class(load(MODE)))
    LES.save_current_level()
    print("saved, game mode", world.get_world_settings().get_editor_property("default_game_mode").get_name())
