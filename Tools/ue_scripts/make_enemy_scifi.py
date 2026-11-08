"""8-5e: 연구소 적 Sci-Fi Soldier 02 메시를 Wasteland 뼈대에 붙인 사본으로 만든다(같은 애니메이션·애님 BP를 쓰기 위해).

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/make_enemy_scifi.py mesh   # 메시 사본
    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/make_enemy_scifi.py bp     # 적 BP

두 팩의 UE4_Mannequin_Skeleton은 본 68개·순서·레퍼런스 포즈가 같다(차이 0cm). 팩 원본은 고치지 않고
메시를 Showcase/Enemy/Meshes로 복제해 뼈대만 Wasteland 것으로 바꾼다. 물리 에셋은 팩 것을 그대로 참조한다(본 이름으로 찾음).
몸 비율은 다르다(솔저가 발 5cm 낮고 팔 8cm 김). 같은 애니메이션으로 나란히 세워 본 결과(compare_scifi.py) 차이가 거의 보이지 않아 공유한다.
bp: Showcase/Enemy/BP_EnemySciFiSoldier = BP_EnemyWasteland 자식, 메시만 교체(애님 BP·적 데이터·캡슐은 물려받음).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

SRC = "/Game/SciFiSoldier02/Meshes/SK_SciFiSoldier02"
MESH = "/Game/Showcase/Enemy/Meshes/SK_SciFiSoldier02_Enemy"
SKELETON = "/Game/WastelandWarrior/meshes/UE4_Mannequin_Skeleton"
BP = "/Game/Showcase/Enemy/BP_EnemySciFiSoldier"
PARENT = "/Game/Showcase/Enemy/BP_EnemyWasteland"

cmd = sys.argv[1] if len(sys.argv) > 1 else "mesh"
if cmd == "mesh":
    if EAL.does_asset_exist(MESH):
        raise RuntimeError(f"{MESH} already exists")
    mesh = EAL.duplicate_asset(SRC, MESH)
    if not mesh:
        raise RuntimeError("duplicate failed")
    mesh.modify()
    # Python에서는 읽기 전용이라 속성 경로로 넣는다
    set_path(mesh, "Skeleton", f"/Script/Engine.Skeleton'{load(SKELETON).get_path_name()}'")
    EAL.save_loaded_asset(mesh, only_if_is_dirty=False)
    m = load(MESH)
    print("mesh", MESH, "skeleton", m.get_editor_property("skeleton").get_path_name(), "PA", m.get_editor_property("physics_asset").get_path_name())

elif cmd == "bp":
    if EAL.does_asset_exist(BP):
        raise RuntimeError(f"{BP} already exists")
    folder, name = BP.rsplit("/", 1)
    f = unreal.BlueprintFactory()
    f.set_editor_property("parent_class", bp_class(load(PARENT)))
    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.Blueprint, f)
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary
    mesh = None
    for h in sds.k2_gather_subobject_data_for_blueprint(bp):
        data = lib.get_data(h)
        if str(lib.get_variable_name(data)) == "Mesh":   # 변수 이름은 속성 이름(오브젝트 이름 CharacterMesh0 아님)
            mesh = lib.get_object_for_blueprint(data, bp)
    if not mesh:
        raise RuntimeError("Mesh component not found")
    mesh.modify()
    mesh.set_skinned_asset_and_update(load(MESH))
    compile_save(bp)
    cdo = unreal.get_default_object(bp_class(bp))
    m = cdo.get_editor_property("mesh")
    print("bp", BP, "mesh", m.get_skinned_asset().get_name(), "anim", m.get_editor_property("anim_class").get_name(),
          "data", cdo.get_component_by_class(unreal.load_class(None, "/Script/FirstVR.DamageReceiver")).get_editor_property("enemy_data").get_name(),
          "PA", m.get_skinned_asset().get_editor_property("physics_asset").get_name())

