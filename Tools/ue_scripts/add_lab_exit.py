"""8-6a: L_Lab 둘째 방 안쪽 끝에 탈출문(게이트)과 그 뒤 짧은 복도를 단다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/add_lab_exit.py [dry]

뒷벽 가운데는 높이 50cm 단(타일 3장, 깊이 10.5m)이라 바닥(0)과 높이가 안 맞는다. 그래서 안쪽 끝 모서리의 긴 벽
(방 A 기준 북쪽 벽 y 1448, x -1425~-725, 폭 7m, 바닥 z 0)을 뺀다. 그 자리에 방 A 입구 복도 끝 묶음
(게이트 + 기둥 2 + 50cm 벽 2, 폭 정확히 7m)과 복도(바닥·옆벽·천장·조명·반사 캡처)를 돌려 붙이고, 복도 끝을 벽으로 막는다.

좌표는 방 A 기준으로 계산한 뒤 둘째 방 변환(게이트 바깥 끝 (1675, 400) 축 180도: x' = 3350 - x, y' = 800 - y, Yaw + 180)으로 옮긴다.
복도 묶음 C(방 A x 690~1680, y 40~760)의 게이트 쪽 평면 x 1400을 벽 평면에 맞추고 복도가 바깥(+Y)으로 나가게 Yaw -90 돌린다:
  u = x - 1400, v = y - 400  →  새 x = -1075 + v, 새 y = 1448 - u.
게이트 틀은 방 안쪽으로 275cm 들어오므로 그 자리의 소품(방 B)은 지운다. 결과는 아웃라이너 폴더 RoomB_Exit. 이미 있으면 멈춘다.
dry: 지울 것·복제할 것만 출력.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
DRY = len(sys.argv) > 1 and sys.argv[1] == "dry"
WALL_Y, CENTER_X = 1448, -1075
SKIP = {"PlayerStart", "PostProcessVolume"}


def b_to_a(v):
    return unreal.Vector(3350 - v.x, 800 - v.y, v.z)


def a_to_b(v):
    return unreal.Vector(3350 - v.x, 800 - v.y, v.z)


def box_a(actor, to_a):
    o, e = actor.get_actor_bounds(False)
    p, q = o - e, o + e
    if to_a:
        p, q = b_to_a(p), b_to_a(q)
    return (min(p.x, q.x), max(p.x, q.x), min(p.y, q.y), max(p.y, q.y), min(p.z, q.z), max(p.z, q.z))


def name(a):
    if isinstance(a, unreal.StaticMeshActor):
        sm = a.static_mesh_component.get_editor_property("static_mesh")
        return sm.get_name() if sm else "?"
    return a.get_class().get_name()


actors = EAS.get_all_level_actors()
if any(str(a.get_folder_path()) == "RoomB_Exit" for a in actors):
    raise RuntimeError("RoomB_Exit already exists")
room_a = [a for a in actors if str(a.get_folder_path()) == "RoomA"]
room_b = [a for a in actors if str(a.get_folder_path()) == "RoomB"]

# 지울 것(방 B): 북쪽 벽 7m 한 장 + 게이트 틀이 들어올 자리(x -1325~-825, y 1173~1448)의 소품(홀로그램·캡슐·난간). 바닥·천장·반사 캡처·조명은 남김
remove = []
for a in room_b:
    x0, x1, y0, y1, z0, z1 = box_a(a, True)
    wall = name(a) == "SM_MWall03-700x350-1" and abs((y0 + y1) / 2 - WALL_Y) < 10 and abs((x0 + x1) / 2 - CENTER_X) < 10
    keep = any(k in name(a) for k in ("Ceiling", "Capture", "Light"))   # 천장·반사 캡처·조명은 남김
    prop = x1 > -1325 and x0 < -825 and y1 > 1180 and y0 < WALL_Y - 10 and z0 < 300 and z1 > 5 and not keep
    if wall or prop:
        remove.append(a)
# 복도 묶음 C(방 A 입구 복도)
corridor = [a for a in room_a if a.get_class().get_name() not in SKIP]
corridor = [a for a in corridor if (lambda b: b[0] > 690 and b[1] < 1680 and b[2] > 40 and b[3] < 760)(box_a(a, False))]
# 복도 끝 막을 벽: 방 A 북쪽 벽 7m(같은 방향)을 +700 옮겨 쓴다
cap = [a for a in room_a if name(a) == "SM_MWall03-700x350-1" and (lambda b: abs((b[2] + b[3]) / 2 - WALL_Y) < 10 and abs((b[0] + b[1]) / 2 - CENTER_X) < 10)(box_a(a, False))]

print("remove", [(a.get_actor_label(), name(a)) for a in remove])
print("corridor", len(corridor), sorted(name(a) for a in corridor))
print("cap", [a.get_actor_label() for a in cap])
if DRY:
    raise SystemExit
if len(cap) != 1:
    raise RuntimeError(f"cap wall not found: {cap}")

world = UES.get_editor_world()
for a in remove:
    EAS.destroy_actor(a)

made = []
for a in EAS.duplicate_actors(corridor, world, unreal.Vector(0, 0, 0)):
    loc = a.get_actor_location()
    u, v = loc.x - 1400, loc.y - 400
    new_a = unreal.Vector(CENTER_X + v, WALL_Y - u, loc.z)
    rot = a.get_actor_rotation()
    a.set_actor_location(a_to_b(new_a), False, False)
    a.set_actor_rotation(unreal.Rotator(rot.roll, rot.pitch, rot.yaw - 90 + 180), False)
    made.append(a)
for a in EAS.duplicate_actors(cap, world, unreal.Vector(0, 0, 0)):
    loc = a.get_actor_location()
    rot = a.get_actor_rotation()
    a.set_actor_location(a_to_b(unreal.Vector(loc.x, loc.y + 700, loc.z)), False, False)
    a.set_actor_rotation(unreal.Rotator(rot.roll, rot.pitch, rot.yaw + 180), False)
    made.append(a)
for a in made:
    a.set_folder_path("RoomB_Exit")
    a.set_actor_label(a.get_actor_label().replace("_B", "") + "_Exit")
gate = [a for a in made if "Gate" in a.get_class().get_name()]
print("made", len(made), "gate at", gate[0].get_actor_location() if gate else None)
LES.save_current_level()
