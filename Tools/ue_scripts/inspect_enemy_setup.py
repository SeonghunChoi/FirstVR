"""8-5c 조사: 지금 적(데이터·BP)과 웨이브·게임 모드, 새 캐릭터 메시의 값을 출력한다(변경 없음)."""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

d = load("/Game/Portfolio/Data/DA_Enemy_Mannequin")
for p in ["max_health", "armor", "bone_multipliers", "left_leg_bones", "right_leg_bones", "limp_threshold", "corpse_life_span",
          "walk_speed", "limp_speed_scale", "attack_range", "attack_montages", "attack_damage", "attack_cooldown"]:
    print("DATA", p, d.get_editor_property(p))

bp = load("/Game/Portfolio/Enemies/BP_EnemyManny")
cdo = unreal.get_default_object(bp_class(bp))
m = cdo.get_editor_property("mesh")
print("MESH", m.get_skinned_asset().get_path_name(), m.get_editor_property("relative_location"), m.get_editor_property("relative_rotation"),
      m.get_editor_property("relative_scale3d"), "anim", m.get_editor_property("anim_class"))
cap = cdo.get_editor_property("capsule_component")
print("CAPSULE", cap.get_unscaled_capsule_half_height(), cap.get_unscaled_capsule_radius())
for c in cdo.get_components_by_class(unreal.ActorComponent):
    print("COMP", c.get_name(), c.get_class().get_name())
    if c.get_class().get_name() == "DamageReceiver":
        print("   enemy data", c.get_editor_property("enemy_data"))
print("AI", cdo.get_editor_property("ai_controller_class"))

for path in ["/Game/WastelandWarrior/meshes/W_warrior_ver1_SK", "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"]:
    sk = load(path)
    b = sk.get_bounds()
    print("BOUNDS", path.split("/")[-1], b.origin, b.box_extent, "PA", sk.get_editor_property("physics_asset").get_path_name())

w = load("/Game/Portfolio/Data/DA_Waves_Combat")
print("WAVES start", w.get_editor_property("start_delay"))
for i, wave in enumerate(w.get_editor_property("waves")):
    print(" wave", i, "rest", wave.get_editor_property("rest_after_clear"))
    for g in wave.get_editor_property("groups"):
        print("   ", g.get_editor_property("enemy_class").get_name(), g.get_editor_property("count"), g.get_editor_property("spawn_point_name"),
              g.get_editor_property("start_delay"), g.get_editor_property("spawn_interval"))

gm = load("/Game/Portfolio/Game/BP_FirstVRCombatMode")
gcdo = unreal.get_default_object(bp_class(gm))
for c in gcdo.get_components_by_class(unreal.ActorComponent):
    print("GM COMP", c.get_name(), c.get_class().get_name())
ws = gcdo.get_component_by_class(unreal.load_class(None, "/Script/FirstVR.WaveSpawner"))
print("GM wave data", ws.get_editor_property("wave_data") if ws else None)

for a in unreal.EditorAssetLibrary.list_assets("/Game/WastelandWarrior", recursive=True):
    if "/meshes/" not in a and ("Material" in a or "Tex" in a):
        continue
    print("PACK", a)
