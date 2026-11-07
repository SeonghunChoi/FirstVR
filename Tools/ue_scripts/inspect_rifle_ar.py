"""8-2d 조사: SK_Rifle의 b_Safety 본과 BP_RifleWeapon_AR의 구성."""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

mesh = load("/Game/FPS_Assault_Pack/Meshes/Rifle/SK_Rifle")
skel = mesh.get_editor_property("skeleton")
print("skeleton", skel.get_path_name())

pose = unreal.AnimPoseExtensions.get_reference_pose(skel)
names = unreal.AnimPoseExtensions.get_bone_names(pose)
print("bones", [str(n) for n in names])
for b in ["b_Root", "b_Safety", "b_Trigger", "b_Mag"]:
    if b in [str(n) for n in names]:
        loc = unreal.AnimPoseExtensions.get_bone_pose(pose, b, unreal.AnimPoseSpaces.LOCAL)
        wld = unreal.AnimPoseExtensions.get_bone_pose(pose, b, unreal.AnimPoseSpaces.WORLD)
        print(b, "local", loc.translation, loc.rotation.rotator(), "| comp", wld.translation, wld.rotation.rotator())
        q = wld.rotation
        print("   axes X", q.get_axis_x(), "Y", q.get_axis_y(), "Z", q.get_axis_z())

bp = load("/Game/Showcase/Weapon/BP_RifleWeapon_AR")
print("parent", unreal.SystemLibrary.get_class_display_name(BEL.generated_class(bp)))
dump_graph(graph(bp), pins=True)
