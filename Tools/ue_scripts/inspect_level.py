"""맵을 열어 구성을 요약한다: 서브레벨, 액터 종류별 개수, 조명, 크기.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/inspect_level.py /Game/.../Map
저장 안 된 맵이 있으면 멈춘다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *
from collections import Counter

path = sys.argv[1]
if unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages():
    raise RuntimeError("Current map has unsaved changes")
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not LES.load_level(path):
    raise RuntimeError(f"Failed to load {path}")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
print("world", world.get_path_name())

for s in unreal.EditorLevelUtils.get_levels(world):
    print("level", s.get_path_name())
streaming = unreal.GameplayStatics.get_streaming_level(world, "") if False else None

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
print("actors", len(actors))
for name, n in Counter(a.get_class().get_name() for a in actors).most_common(25):
    print(f"  {n:5d} {name}")

lo = unreal.Vector(1e9, 1e9, 1e9); hi = unreal.Vector(-1e9, -1e9, -1e9)
for a in actors:
    if isinstance(a, unreal.StaticMeshActor):
        o, e = a.get_actor_bounds(False)
        lo = unreal.Vector(min(lo.x, o.x - e.x), min(lo.y, o.y - e.y), min(lo.z, o.z - e.z))
        hi = unreal.Vector(max(hi.x, o.x + e.x), max(hi.y, o.y + e.y), max(hi.z, o.z + e.z))
print("static mesh bounds (m)", round((hi.x - lo.x) / 100), "x", round((hi.y - lo.y) / 100), "x", round((hi.z - lo.z) / 100), "from", lo, "to", hi)

for a in actors:
    if isinstance(a, (unreal.DirectionalLight, unreal.SkyLight)):
        c = a.get_component_by_class(unreal.LightComponentBase)
        print("light", a.get_actor_label(), a.get_class().get_name(), "mobility", c.get_editor_property("mobility"))
    if a.get_class().get_name() in ("PostProcessVolume",):
        print("postprocess", a.get_actor_label(), "unbound", a.get_editor_property("unbound"))
