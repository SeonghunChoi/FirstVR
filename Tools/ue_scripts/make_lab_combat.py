"""8-6d: L_Lab 전투 배치 — 전투 구간마다 무기 책상·탄창 책상, 앞쪽 좌·우 스폰 지점, 구간별 웨이브 2개.

    python Tools/ue_remote.py Tools/ue_scripts/make_lab_combat.py data    # 웨이브 데이터 DA_Waves_Lab_S1~S3 다시 쓰기
    python Tools/ue_remote.py Tools/ue_scripts/make_lab_combat.py level   # L_Lab 배치(열려 있어야 함)

결정(2026-11-09): 홀스터가 없고 되돌아갈 수 없어 전투 구간마다 꾸민 무기 세 종 + 탄창 보급대 세 개,
스폰 지점은 구간이 바라보는 쪽 약 10m 앞 좌·우, 웨이브는 구간마다 2개(쉼 5초).
- 스폰 지점은 구간마다 앞 거리·좌우 거리를 정한다(내비 메시 위여야 함).
- 설 자리(표식 가운데)의 오른쪽에 무기 책상, 왼쪽에 탄창 책상(SM_LabDesk03_2, 깊이 125·너비 240·높이 90).
  책상의 긴 쪽이 바라보는 방향을 따라 놓이고, 가까운 모서리는 설 자리에서 옆으로 DESK_GAP.
- 구간 방향(액터 Yaw)을 정해진 값으로 돌린다(도착하면 적 쪽을 본다).
level은 다시 실행하면 폴더 Gameplay/Combat의 이전 결과와 옛 스폰 지점(Lab_S1~S3)을 지우고 새로 놓는다. 내비 메시를 다시 만들고 저장.
"""
import sys, math; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

cmd = sys.argv[1] if len(sys.argv) > 1 else "level"
SOLDIER = "/Game/Showcase/Enemy/BP_EnemySciFiSoldier"
SHOW = "/Game/Showcase/Weapon/"
# 전투 구간: (라벨, 웨이브 데이터 번호, 바라보는 방향 Yaw, 스폰 앞 거리, 스폰 좌우 거리)
# 첫 방 가운데 줄(x -700~-1000, y 100~300)에는 소품이 있어 구간 1은 더 멀리·넓게 둔다.
FIGHTS = [("Section_1_Fight", 1, 180.0, 1200.0, 500.0), ("Section_3_Fight", 2, 180.0, 1000.0, 300.0),
          ("Section_9_Fight", 3, 0.0, 1000.0, 300.0)]
DESK_GAP = 75.0   # 책상 사이 1.5m: 적 반지름 35cm를 빼고도 설 자리에 내비 메시가 남아야 순간이동이 겨눠진다
# 웨이브: [(왼쪽 수, 오른쪽 수), ...]
WAVES = {1: [(1, 1), (1, 1)], 2: [(1, 1), (2, 1)], 3: [(2, 1), (3, 2)]}
FOLDER = "Gameplay/Combat"

if cmd == "data":
    for n, waves in WAVES.items():
        wd = load(f"/Game/Showcase/Game/DA_Waves_Lab_S{n}")
        defs = []
        for left, right in waves:
            groups = []
            for side, count, delay in (("L", left, 0.0), ("R", right, 1.5)):
                if count <= 0:
                    continue
                g = unreal.WaveSpawnGroup()
                g.set_editor_property("enemy_class", bp_class(load(SOLDIER)))
                g.set_editor_property("count", count)
                g.set_editor_property("spawn_point_name", f"Lab_F{n}_{side}")
                g.set_editor_property("start_delay", delay)
                g.set_editor_property("spawn_interval", 3.0)
                groups.append(g)
            w = unreal.WaveDefinition()
            w.set_editor_property("groups", groups)
            w.set_editor_property("rest_after_clear", 5.0)
            defs.append(w)
        wd.set_editor_property("start_delay", 2.0)
        wd.set_editor_property("waves", defs)
        EAL.save_loaded_asset(wd, only_if_is_dirty=False)
        print(wd.get_name(), [(sum(g.get_editor_property("count") for g in w.get_editor_property("groups"))) for w in defs])
    raise SystemExit

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
NS = unreal.NavigationSystemV1
world = UES.get_editor_world()
if world is None or world.get_path_name().split(".")[0] != "/Game/Showcase/Map/L_Lab":
    raise RuntimeError("open L_Lab first (and stop Play)")

actors = EAS.get_all_level_actors()
for a in actors:
    old_spawn = a.get_class().get_name() == "EnemySpawnPoint" and str(a.get_editor_property("spawn_point_name")).startswith("Lab_S")
    if str(a.get_folder_path()).startswith(FOLDER) or old_spawn:
        EAS.destroy_actor(a)
actors = EAS.get_all_level_actors()
by_label = {a.get_actor_label(): a for a in actors}

desk_mesh = load("/Game/SciFiWorld/Meshes/SM_LabDesk03_2")
disp_cls = load_cls("/Game/Portfolio/Weapons/Magazines/BP_MagazineDispenser.BP_MagazineDispenser_C")
sp_cls = load_cls("/Script/FirstVR.EnemySpawnPoint")
ms_cls = load_cls("/Script/FirstVR.MagazineStep")
KIT = [("BP_RifleWeapon_AR", "BP_Magazine_AR"), ("BP_AutoRifleWeapon_RU74", "BP_Magazine_RU74"), ("BP_PistolWeapon_PM", "BP_Magazine_PM")]


def vec(origin, fwd, right, f, r, z):
    return unreal.Vector(origin.x + fwd.x * f + right.x * r, origin.y + fwd.y * f + right.y * r, origin.z + z)


def on_nav(p):
    q = NS.project_point_to_navigation(world, p, None, None, unreal.Vector(50, 50, 200))
    return q


for label, n, yaw, SPAWN_AHEAD, SPAWN_SIDE in FIGHTS:
    sec = by_label[label]
    sec.set_actor_rotation(unreal.Rotator(0, 0, yaw), False)
    c = sec.get_actor_location()
    fwd = unreal.Vector(math.cos(math.radians(yaw)), math.sin(math.radians(yaw)), 0)
    right = unreal.Vector(-fwd.y, fwd.x, 0)          # UE는 Y가 오른쪽(왼손 좌표계)
    floor = c.z

    # 책상: 오른쪽 = 무기, 왼쪽 = 탄창. 긴 쪽(로컬 Y)이 앞뒤, 로컬 X(깊이 0~125)가 바깥쪽을 향한다.
    for side, sign in (("Weapons", 1.0), ("Magazines", -1.0)):
        out = unreal.Vector(right.x * sign, right.y * sign, 0)
        desk_yaw = math.degrees(math.atan2(out.y, out.x))
        d = EAS.spawn_actor_from_object(desk_mesh, vec(c, fwd, out, 0, DESK_GAP, 0), unreal.Rotator(0, 0, desk_yaw))
        d.set_actor_label(f"F{n}_Desk_{side}")
        d.set_folder_path(FOLDER)
        if side == "Weapons":
            for i, (weapon, mag) in enumerate(KIT):
                p = vec(c, fwd, out, (i - 1) * 75.0, DESK_GAP + 55.0, 105.0)
                w = EAS.spawn_actor_from_class(load_cls(f"{SHOW}{weapon}.{weapon}_C"), p, unreal.Rotator(-90, 0, desk_yaw))
                w.set_actor_label(f"F{n}_{weapon}")
                w.set_folder_path(FOLDER)
                w.get_component_by_class(ms_cls).set_editor_property("InitialMagazine", load_cls(f"{SHOW}{mag}.{mag}_C"))
                o, e = w.get_actor_bounds(False)   # 놓인 무기는 잡기 전엔 물리로 떨어지지 않으니 바닥면을 책상 윗면(+0.5)에
                l = w.get_actor_location()
                w.set_actor_location(unreal.Vector(l.x, l.y, l.z - (o.z - e.z - (floor + 90)) + 0.5), False, False)
        else:
            for i, (weapon, mag) in enumerate(KIT):
                p = vec(c, fwd, out, (i - 1) * 70.0, DESK_GAP + 40.0, 90.0)
                disp = EAS.spawn_actor_from_class(disp_cls, p, unreal.Rotator(0, 0, desk_yaw))
                disp.set_actor_label(f"F{n}_Dispenser_{mag}")
                disp.set_folder_path(FOLDER)
                disp.set_editor_property("MagazineClass", load_cls(f"{SHOW}{mag}.{mag}_C"))

    # 스폰 지점: 앞쪽 SPAWN_AHEAD, 좌·우 SPAWN_SIDE. 내비 메시 위로 맞추고 설 자리를 바라보게.
    for side, sign in (("L", -1.0), ("R", 1.0)):
        p = on_nav(vec(c, fwd, right, SPAWN_AHEAD, sign * SPAWN_SIDE, 0))
        if p is None:
            raise RuntimeError(f"F{n}_{side} spawn not on nav mesh")
        s = EAS.spawn_actor_from_class(sp_cls, unreal.Vector(p.x, p.y, p.z - 10), unreal.Rotator(0, 0, yaw + 180))
        s.set_editor_property("spawn_point_name", f"Lab_F{n}_{side}")
        s.set_actor_label(f"SpawnPoint_Lab_F{n}_{side}")
        s.set_folder_path(FOLDER)
        print(f"F{n} spawn {side}", (round(p.x), round(p.y), round(p.z)))
    print(label, "yaw", yaw, "center", (round(c.x), round(c.y)))

unreal.SystemLibrary.execute_console_command(world, "RebuildNavigation")
print("placed; rebuild navigation requested")
