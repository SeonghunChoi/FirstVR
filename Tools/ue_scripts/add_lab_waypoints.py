"""8-6c: L_Lab 이동 지점 — 순간이동 포물선이 편하게 닿는 거리(약 4.5m 이하)마다 웨이브 없는 구간을 둔다.

    python Tools/ue_remote.py Tools/ue_scripts/add_lab_waypoints.py   # L_Lab이 열려 있어야 함(Play 중이면 멈춤)

템플릿 순간이동(발사 속도 650cm/s)은 손 높이 1m·45도에서 평평한 바닥 최대 약 5m다.
웨이브 없는 구간은 도착하는 순간 끝나고 다음 표식이 켜진다(회복 0, 다음 자리 쪽을 봄).
다시 실행하면 기존 이동 지점(웨이브 없는 구간 중 마지막 탈출 지점을 뺀 것)을 지우고 새로 놓는다.
게이트(x 1275·1925, 탈출문 y -623)의 문틀 안과 둘째 방 단·칸막이는 피한다. 놓기 전에 내비 메시 위인지, 다음 자리까지 1m 높이 시야가 트였는지(문 제외) 본다.
"""
import sys, math; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
NS = unreal.NavigationSystemV1
world = UES.get_editor_world()
if world is None or world.get_path_name().split(".")[0] != "/Game/Showcase/Map/L_Lab":
    raise RuntimeError("open L_Lab first (and stop Play)")

# 전투 구간(웨이브 있음)의 웨이브 데이터 → 그 앞에 넣을 이동 지점. 키 None은 마지막 탈출 지점 앞.
WAYPOINTS_BEFORE = {
    "DA_Waves_Lab_S2": [(600, 400)],
    "DA_Waves_Lab_S3": [(1200, 400), (1600, 400), (2000, 400), (2450, 400), (2900, 400)],
    # 둘째 방: 뒷벽 쪽 단(높이 50, y 47~753, 난간)과 남쪽 칸막이(y -650~-300) 사이 통로(y 약 -130)로 간다
    None: [(3450, 0), (3850, -130), (4250, -130), (4425, -450), (4425, -750)],
}

actors = EAS.get_all_level_actors()
sections = sorted((a for a in actors if a.get_class().get_name().startswith("BP_EncounterSection")),
                  key=lambda a: a.get_editor_property("section_order"))
escape = sections[-1]
fights = [s for s in sections if s.get_editor_property("wave_data")]
old_moves = [s for s in sections if not s.get_editor_property("wave_data") and s is not escape]
gates = [a for a in actors if "Gate" in a.get_class().get_name()]

for (x, y) in [p for ps in WAYPOINTS_BEFORE.values() for p in ps]:
    if NS.project_point_to_navigation(world, unreal.Vector(x, y, 0), None, None, unreal.Vector(30, 30, 200)) is None:
        raise RuntimeError(f"({x}, {y}) is not on the nav mesh")

for s in old_moves:
    EAS.destroy_actor(s)
print("removed", len(old_moves), "old waypoints")

sec_cls = bp_class(load("/Game/Portfolio/Game/BP_EncounterSection"))


def spawn_move(x, y):
    w = EAS.spawn_actor_from_class(sec_cls, unreal.Vector(x, y, 0))
    w.set_editor_property("heal_on_clear", 0.0)
    w.set_editor_property("doors_to_unlock", [])
    w.set_folder_path("Gameplay")
    return w


ordered = []
for s in fights:
    ordered += [spawn_move(x, y) for (x, y) in WAYPOINTS_BEFORE.get(s.get_editor_property("wave_data").get_name(), [])]
    ordered.append(s)
ordered += [spawn_move(x, y) for (x, y) in WAYPOINTS_BEFORE[None]]
ordered.append(escape)

for i, s in enumerate(ordered):
    s.set_editor_property("section_order", i + 1)
    kind = "Fight" if s.get_editor_property("wave_data") else ("Escape" if s is escape else "Move")
    s.set_actor_label(f"Section_{i + 1}_{kind}")
    if kind == "Move":   # 다음 자리 쪽을 본다
        a, b = s.get_actor_location(), ordered[i + 1].get_actor_location()
        s.set_actor_rotation(unreal.Rotator(0, 0, math.degrees(math.atan2(b.y - a.y, b.x - a.x))), False)

ignore = gates + ordered
for i, s in enumerate(ordered):
    a = s.get_actor_location()
    hop = ""
    if i + 1 < len(ordered):
        b = ordered[i + 1].get_actor_location()
        hit = unreal.SystemLibrary.line_trace_single(world, a + unreal.Vector(0, 0, 100), b + unreal.Vector(0, 0, 100),
                                                     unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, ignore, unreal.DrawDebugTrace.NONE, True)
        blocked = hit.to_tuple()[9].get_actor_label() if hit else None
        hop = f"-> {math.hypot(b.x - a.x, b.y - a.y):.0f} cm" + (f" BLOCKED by {blocked}" if blocked else "")
    print(s.get_actor_label(), (round(a.x), round(a.y)), round(s.get_actor_rotation().yaw), hop)

LES.save_current_level()
print("saved")
