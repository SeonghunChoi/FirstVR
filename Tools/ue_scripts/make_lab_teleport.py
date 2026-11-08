"""8-6c: L_Lab 순간이동 준비 — 구간 방향(도착한 플레이어가 볼 쪽)과 내비 메시.

    python Tools/ue_remote.py Tools/ue_scripts/make_lab_teleport.py   # L_Lab이 열려 있어야 함

- 구간 1~3은 그 구간 웨이브의 스폰 지점 쪽, 구간 4(탈출)는 탈출 복도 끝(-Y) 쪽을 보게 액터 Yaw를 돌린다.
- 내비 메시: 바닥 메시(라벨에 Floor) 전체를 덮는 NavMeshBoundsVolume. 템플릿 순간이동은 내비 메시 위만 겨눌 수 있고,
  8-6d의 적 이동에도 필요하다. 이미 있으면 크기만 다시 맞춘다. 경로가 잡힌 뒤 저장.
"""
import sys, math; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world = UES.get_editor_world()
if world.get_path_name().split(".")[0] != "/Game/Showcase/Map/L_Lab":
    raise RuntimeError("open L_Lab first")

actors = EAS.get_all_level_actors()
spawns = {str(a.get_editor_property("spawn_point_name")): a for a in actors if a.get_class().get_name() == "EnemySpawnPoint"}
sections = sorted((a for a in actors if a.get_class().get_name().startswith("BP_EncounterSection")),
                  key=lambda a: a.get_editor_property("section_order"))

for s in sections:
    loc = s.get_actor_location()
    wd = s.get_editor_property("wave_data")
    target = None
    if wd:
        names = {str(g.get_editor_property("spawn_point_name")) for w in wd.get_editor_property("waves") for g in w.get_editor_property("groups")}
        pts = [spawns[n].get_actor_location() for n in names if n in spawns]
        if pts:
            target = unreal.Vector(sum(p.x for p in pts) / len(pts), sum(p.y for p in pts) / len(pts), 0)
    yaw = math.degrees(math.atan2(target.y - loc.y, target.x - loc.x)) if target else -90.0
    s.set_actor_rotation(unreal.Rotator(0, 0, yaw), False)
    print(s.get_actor_label(), "yaw", round(yaw, 1), "toward", target)

# 내비 메시 범위
floors = [a for a in actors if "Floor" in a.get_actor_label() and isinstance(a, unreal.StaticMeshActor)]
lo = [1e9, 1e9]; hi = [-1e9, -1e9]
for f in floors:
    o, e = f.get_actor_bounds(False)
    lo = [min(lo[0], o.x - e.x), min(lo[1], o.y - e.y)]
    hi = [max(hi[0], o.x + e.x), max(hi[1], o.y + e.y)]
print("floors", len(floors), "x", lo[0], hi[0], "y", lo[1], hi[1])

nav = next((a for a in actors if isinstance(a, unreal.NavMeshBoundsVolume)), None)
center = unreal.Vector((lo[0] + hi[0]) / 2, (lo[1] + hi[1]) / 2, 100)
if nav is None:
    nav = EAS.spawn_actor_from_object(unreal.NavMeshBoundsVolume.static_class(), center, unreal.Rotator(0, 0, 0))
    nav.set_actor_label("NavMeshBoundsVolume")
    nav.set_folder_path("Gameplay")
nav.set_actor_scale3d(unreal.Vector(1, 1, 1))
nav.set_actor_location(center, False, False)
o, e = nav.get_actor_bounds(False)
nav.set_actor_scale3d(unreal.Vector((hi[0] - lo[0]) / 2 / e.x, (hi[1] - lo[1]) / 2 / e.y, 250 / e.z))
print("nav bounds", nav.get_actor_bounds(False))

unreal.SystemLibrary.execute_console_command(world, "RebuildNavigation")
print("rebuild requested; save after paths are found")
