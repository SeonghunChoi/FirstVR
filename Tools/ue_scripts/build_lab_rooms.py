"""8-6a: L_Lab(팩 방 A를 다른 이름으로 저장한 맵)에 둘째 방을 이어 붙인다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/build_lab_rooms.py

방 A 구조: 본실 x -1425~700(21m) + 입구 복도 x 700~1400(폭 7m, y 50~750) + 게이트 틀 x 1175~1675(가운데 x 1425, y 400).
방 A의 액터를 복제해 게이트 바깥 끝(1675, 400)을 축으로 180도 돌린다 → 둘째 방의 게이트 틀이 1675~2175에 붙어
두 게이트를 지나 복도-복도로 이어지고, 둘째 방 본실은 x 2650~4775.
복제하지 않는 것: PlayerStart, PostProcessVolume(하나를 두 방에 맞게 늘림), InstancedFoliageActor(인스턴스는 복제가 따로 필요).
복제본은 아웃라이너 폴더 RoomB, 원본은 RoomA로 정리한다. 이미 RoomB가 있으면 멈춘다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
MAP = "/Game/Showcase/Map/L_Lab"
PIVOT = unreal.Vector(1675, 400, 0)
SKIP = {"PlayerStart", "PostProcessVolume", "InstancedFoliageActor", "WorldSettings", "Brush", "DefaultPhysicsVolume",
        "WorldDataLayers", "WorldPartitionMiniMap", "LevelBounds", "AbstractNavData", "NavigationData"}

world = UES.get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError(f"open {MAP} first (now {world.get_path_name()})")
actors = EAS.get_all_level_actors()
if any(str(a.get_folder_path()) == "RoomB" for a in actors):
    raise RuntimeError("RoomB already exists")

src = [a for a in actors if a.get_class().get_name() not in SKIP and not a.get_class().get_name().startswith("Default")]
for a in src:
    a.set_folder_path("RoomA")
copies = EAS.duplicate_actors(src, world, unreal.Vector(0, 0, 0))
for a in copies:
    loc = a.get_actor_location()
    rel = loc - PIVOT
    a.set_actor_location(unreal.Vector(PIVOT.x - rel.x, PIVOT.y - rel.y, loc.z), False, False)
    rot = a.get_actor_rotation()
    a.set_actor_rotation(unreal.Rotator(rot.roll, rot.pitch, rot.yaw + 180), False)
    a.set_folder_path("RoomB")
    a.set_actor_label(a.get_actor_label() + "_B")
print("copied", len(copies), "of", len(src))

pp = [a for a in actors if a.get_class().get_name() == "PostProcessVolume"]
for v in pp:
    o, e = v.get_actor_bounds(False)
    print("postprocess bounds", o, e)

LES.save_current_level()
print("saved", world.get_path_name())
