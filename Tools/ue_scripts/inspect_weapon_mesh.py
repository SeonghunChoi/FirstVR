"""스켈레탈 메시 본 위치·축과 무기 BP 이벤트 그래프를 출력한다.

    python Tools/ue_remote.py Tools/ue_scripts/inspect_weapon_mesh.py <메시 경로> <무기 BP 경로> [본 이름...]
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

mesh_path, bp_path = sys.argv[1], sys.argv[2]
watch = sys.argv[3:]
mesh = load(mesh_path)
skel = mesh.get_editor_property("skeleton")
print("skeleton", skel.get_path_name())
pose = unreal.AnimPoseExtensions.get_reference_pose(skel)
names = [str(n) for n in unreal.AnimPoseExtensions.get_bone_names(pose)]
print("bones", names)
for b in watch:
    if b not in names:
        print(b, "missing"); continue
    loc = unreal.AnimPoseExtensions.get_bone_pose(pose, b, unreal.AnimPoseSpaces.LOCAL)
    wld = unreal.AnimPoseExtensions.get_bone_pose(pose, b, unreal.AnimPoseSpaces.WORLD)
    q = wld.rotation
    v = lambda x: f"({x.x:.2f}, {x.y:.2f}, {x.z:.2f})"
    print(f"{b}: local {v(loc.translation)} {loc.rotation.rotator()} | comp {v(wld.translation)} axes X{v(q.get_axis_x())} Y{v(q.get_axis_y())} Z{v(q.get_axis_z())}")
bp = load(bp_path)
dump_graph(graph(bp), pins=False)
