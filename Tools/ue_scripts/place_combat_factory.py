"""8-3b: L_Combat_Factory의 A 마당에 L_Combat 게임 배치를 놓는다.

L_Combat(16m 방, 가운데 원점, 플레이어는 +X를 봄)의 테이블·보급대 배치를 플레이어 기준으로 옮겨,
여기서는 플레이어가 동쪽(+Y)을 본다. 스폰 지점은 마당의 실제 출입구에 둔다(이름은 웨이브 데이터와 같게 Front·Left·Right).
테이블은 공장 팔레트(86×120×15cm)를 두 줄 6단으로 쌓아 높이 90cm. 게임 모드는 BP_FirstVRCombatMode.
다시 실행하면 이 스크립트가 만든 것(폴더 Gameplay)을 지우고 새로 놓는다.
"""
import math
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

FLOOR = 100.0
PLAYER = unreal.Vector(1300, -2400, FLOOR)
YAW = 90.0                                   # 플레이어가 보는 방향(+Y)
CENTER = PLAYER + unreal.Vector(0, 80, 0)   # L_Combat 원점에 해당(플레이어 앞 80cm)
SPAWNS = {"Front": (1280, -500), "Left": (2200, -600), "Right": (200, -2860)}
FOLDER = "Gameplay"

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
LES.set_current_level_by_name("L_Combat_Factory")


def local(lx, ly, z):
    """L_Combat 좌표(원점 기준, +X 앞) → 이 레벨 월드 좌표."""
    r = math.radians(YAW)
    return unreal.Vector(CENTER.x + lx * math.cos(r) - ly * math.sin(r),
                         CENTER.y + lx * math.sin(r) + ly * math.cos(r), FLOOR + z)


def spawn(cls, loc, yaw=0.0, label=None):
    a = EAS.spawn_actor_from_class(cls, loc, unreal.Rotator(0, 0, yaw))
    if label:
        a.set_actor_label(label)
    a.set_folder_path(FOLDER)
    return a


# 이전 실행 결과 지우기
for a in EAS.get_all_level_actors():
    if a.get_level().get_outer().get_name() == "L_Combat_Factory" and str(a.get_folder_path()).startswith(FOLDER)             and a.get_class().get_name() != "PlayerStart":
        EAS.destroy_actor(a)

# 플레이어 시작(템플릿에서 온 것을 옮긴다)
starts = [a for a in EAS.get_all_level_actors() if a.get_class().get_name() == "PlayerStart"
          and a.get_level().get_outer().get_name() == "L_Combat_Factory"]
ps = starts[0] if starts else EAS.spawn_actor_from_class(unreal.PlayerStart, PLAYER, unreal.Rotator(0, 0, YAW))
ps.set_actor_location_and_rotation(PLAYER, unreal.Rotator(0, 0, YAW), False, False)
ps.set_folder_path(FOLDER)

# 팔레트 테이블: L_Combat 테이블(중심 60,0, 깊이 80 × 폭 240, 높이 90)
pallet = load("/Game/AbandonedFactory/Environment/WoodPallet_01/sm_WoodPallet_01_01")
pb = pallet.get_bounds()
pivot_to_bottom = pb.origin.z - pb.box_extent.z      # 메시 원점에서 바닥까지
for side, ly in (("L", -60), ("R", 60)):
    for i in range(6):
        loc = local(60, ly, 15 * i - pivot_to_bottom)
        a = EAS.spawn_actor_from_object(pallet, loc, unreal.Rotator(0, 0, YAW + (2 if i % 2 else -2)))
        a.set_actor_label(f"Table_Pallet_{side}{i}")
        a.set_folder_path(FOLDER + "/Table")

# 탄창 보급대(테이블 위, 높이 90)
disp_cls = load_cls("/Game/Portfolio/Weapons/Magazines/BP_MagazineDispenser.BP_MagazineDispenser_C")
for label, lx, ly, mag in (("BP_MagazineDispenser_Rifle", 60, -40, "/Game/Portfolio/Weapons/Magazines/BP_Magazine_Rifle.BP_Magazine_Rifle_C"),
                           ("BP_MagazineDispenser_Pistol", 61, 40, "/Game/Portfolio/Weapons/Magazines/BP_Magazine_Pistol.BP_Magazine_Pistol_C")):
    d = spawn(disp_cls, local(lx, ly, 90), YAW, label)
    d.set_editor_property("MagazineClass", load_cls(mag))

# 적 스폰 지점: 출입구에서 마당 가운데를 본다
sp_cls = load_cls("/Script/FirstVR.EnemySpawnPoint")
for name, (x, y) in SPAWNS.items():
    yaw = math.degrees(math.atan2(CENTER.y - y, CENTER.x - x))
    s = spawn(sp_cls, unreal.Vector(x, y, FLOOR), yaw, f"EnemySpawnPoint_{name}")
    s.set_editor_property("SpawnPointName", name)

# 내비 메시 범위: X -300~2700, Y -3300~300, Z 0~400
nav = EAS.spawn_actor_from_object(unreal.NavMeshBoundsVolume.static_class(), unreal.Vector(1200, -1500, 200), unreal.Rotator(0, 0, 0))
nav.set_actor_label("NavMeshBoundsVolume")
nav.set_folder_path(FOLDER)
o, e = nav.get_actor_bounds(False)
nav.set_actor_scale3d(unreal.Vector(1500 / e.x, 1800 / e.y, 200 / e.z))
print("nav bounds", nav.get_actor_bounds(False))

world.get_world_settings().set_editor_property(
    "default_game_mode", load_cls("/Game/Portfolio/Game/BP_FirstVRCombatMode.BP_FirstVRCombatMode_C"))
LES.save_current_level()
for a in EAS.get_all_level_actors():
    if str(a.get_folder_path()).startswith(FOLDER) and "Pallet" not in a.get_actor_label():
        print(a.get_class().get_name(), a.get_actor_label(), a.get_actor_location(), round(a.get_actor_rotation().yaw))
