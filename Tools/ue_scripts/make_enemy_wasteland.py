"""8-5c: 공장 적 Wasteland Warrior.

- Showcase/Enemy/DA_Enemy_Wasteland: DA_Enemy_Mannequin 복제. UE4 뼈대에 없는 neck_02를 부위 배율에서 빼고, 공격 몽타주를 _UE4로.
- Showcase/Enemy/BP_EnemyWasteland: BP_EnemyManny 자식. 메시 W_warrior_ver1_SK(물리 에셋은 팩 W_warrior_PA), 애님 BP ABP_Enemy_UE4,
  DamageReceiver 데이터 DA_Enemy_Wasteland. 캡슐·메시 위치·AI 컨트롤러는 부모 그대로.
- Showcase/Game/DA_Waves_Factory: DA_Waves_Combat 복제, 적 클래스만 BP_EnemyWasteland.
- Showcase/Game/BP_FirstVRFactoryMode: BP_FirstVRCombatMode 자식, 웨이브 데이터만 DA_Waves_Factory.
L_Combat_Factory의 게임 모드 오버라이드는 set_factory_mode.py에서 바꾼다(맵을 열어야 해서 따로).
이미 있으면 멈춘다(지우고 다시 할 때는 에셋을 먼저 지운다).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

ENEMY = "/Game/Showcase/Enemy"
GAME = "/Game/Showcase/Game"
DATA = f"{ENEMY}/DA_Enemy_Wasteland"
BP = f"{ENEMY}/BP_EnemyWasteland"
WAVES = f"{GAME}/DA_Waves_Factory"
MODE = f"{GAME}/BP_FirstVRFactoryMode"
for p in (DATA, BP, WAVES, MODE):
    if EAL.does_asset_exist(p):
        raise RuntimeError(f"{p} already exists")

tools = unreal.AssetToolsHelpers.get_asset_tools()
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary


def child_bp(path, parent_path):
    folder, name = path.rsplit("/", 1)
    f = unreal.BlueprintFactory()
    f.set_editor_property("parent_class", bp_class(load(parent_path)))
    return tools.create_asset(name, folder, unreal.Blueprint, f)


def template_of(bp, var_name):
    """자식 BP에서 물려받은 컴포넌트를 덮어쓸 템플릿(C++ 컴포넌트면 CDO의 것)."""
    for h in sds.k2_gather_subobject_data_for_blueprint(bp):
        data = lib.get_data(h)
        if str(lib.get_variable_name(data)) == var_name:
            t = lib.get_object_for_blueprint(data, bp)
            t.modify()
            return t
    raise RuntimeError(f"{var_name} not found in {bp.get_name()}")


# --- 적 데이터
data = EAL.duplicate_asset("/Game/Portfolio/Data/DA_Enemy_Mannequin", DATA)
bones = data.get_editor_property("bone_multipliers")
skeleton_bones = {str(n) for n in unreal.AnimPoseExtensions.get_bone_names(
    unreal.AnimPoseExtensions.get_reference_pose(load("/Game/WastelandWarrior/meshes/UE4_Mannequin_Skeleton")))}
kept = unreal.Map(unreal.Name, float)
for k, v in bones.items():
    if str(k) in skeleton_bones:
        kept[k] = v
    else:
        print("drop bone multiplier", k, v)
data.set_editor_property("bone_multipliers", kept)
for prop in ("left_leg_bones", "right_leg_bones"):
    missing = [str(b) for b in data.get_editor_property(prop) if str(b) not in skeleton_bones]
    if missing:
        raise RuntimeError(f"{prop} not in skeleton: {missing}")
data.set_editor_property("attack_montages", [load(f"{ENEMY}/Anims/AM_Attack_0{i}_UE4") for i in (1, 2, 3)])
EAL.save_loaded_asset(data, only_if_is_dirty=False)
print("data", DATA, data.get_editor_property("bone_multipliers"), [m.get_name() for m in data.get_editor_property("attack_montages")])

# --- 적 BP
bp = child_bp(BP, "/Game/Portfolio/Enemies/BP_EnemyManny")
mesh = template_of(bp, "Mesh")
mesh.set_skinned_asset_and_update(load("/Game/WastelandWarrior/meshes/W_warrior_ver1_SK"))
mesh.set_editor_property("anim_class", bp_class(load(f"{ENEMY}/ABP_Enemy_UE4")))
receiver = template_of(bp, "DamageReceiver")
receiver.set_editor_property("enemy_data", data)
compile_save(bp)
cdo = unreal.get_default_object(bp_class(bp))
m = cdo.get_editor_property("mesh")
print("bp mesh", m.get_skinned_asset().get_name(), "anim", m.get_editor_property("anim_class").get_name(),
      "loc", m.get_editor_property("relative_location"), "rot", m.get_editor_property("relative_rotation"),
      "data", cdo.get_component_by_class(unreal.load_class(None, "/Script/FirstVR.DamageReceiver")).get_editor_property("enemy_data").get_name())

# --- 웨이브 데이터
waves = EAL.duplicate_asset("/Game/Portfolio/Data/DA_Waves_Combat", WAVES)
enemy_cls = bp_class(bp)
new_waves = []
for wave in waves.get_editor_property("waves"):
    groups = []
    for g in wave.get_editor_property("groups"):
        g.set_editor_property("enemy_class", enemy_cls)
        groups.append(g)
    wave.set_editor_property("groups", groups)
    new_waves.append(wave)
waves.set_editor_property("waves", new_waves)
EAL.save_loaded_asset(waves, only_if_is_dirty=False)
for i, wave in enumerate(waves.get_editor_property("waves")):
    print("wave", i, [(g.get_editor_property("enemy_class").get_name(), g.get_editor_property("count"), str(g.get_editor_property("spawn_point_name")))
                      for g in wave.get_editor_property("groups")])

# --- 게임 모드
mode = child_bp(MODE, "/Game/Portfolio/Game/BP_FirstVRCombatMode")
spawner = template_of(mode, "WaveSpawner")
spawner.set_editor_property("wave_data", waves)
compile_save(mode)
gcdo = unreal.get_default_object(bp_class(mode))
print("mode wave data", gcdo.get_component_by_class(unreal.load_class(None, "/Script/FirstVR.WaveSpawner")).get_editor_property("wave_data").get_name())
