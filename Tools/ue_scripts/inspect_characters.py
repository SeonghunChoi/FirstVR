"""8-5 조사: 폴더 안의 스켈레탈 메시마다 뼈대·본 목록·물리 에셋·크기를 출력하고, UE5 마네킹(SKM_Manny_Simple)과 본 이름을 비교한다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/inspect_characters.py /Game/WastelandWarrior /Game/SciFiSoldier02
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

ar = unreal.AssetRegistryHelpers.get_asset_registry()


def bones_of(skeleton):
    pose = unreal.AnimPoseExtensions.get_reference_pose(skeleton)
    return [str(n) for n in unreal.AnimPoseExtensions.get_bone_names(pose)]


manny = load("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple")
manny_bones = set(bones_of(manny.get_editor_property("skeleton")))

for folder in sys.argv[1:]:
    print("=" * 8, folder)
    for a in ar.get_assets_by_path(folder, recursive=True):
        cls = str(a.asset_class_path.asset_name)
        if cls not in ("SkeletalMesh", "Skeleton", "PhysicsAsset", "AnimBlueprint"):
            continue
        obj = a.get_asset()
        if cls == "SkeletalMesh":
            sk = obj.get_editor_property("skeleton")
            pa = obj.get_editor_property("physics_asset")
            b = obj.get_bounds()
            print(f"MESH {a.package_name}  skeleton={sk.get_path_name() if sk else None}  physics={pa.get_path_name() if pa else None}  "
                  f"size(cm)={round(b.box_extent.x*2)}x{round(b.box_extent.y*2)}x{round(b.box_extent.z*2)}  materials={len(obj.get_editor_property('materials'))}")
        elif cls == "Skeleton":
            bones = bones_of(obj)
            only_here = [x for x in bones if x not in manny_bones]
            missing = sorted(x for x in manny_bones if x not in bones)
            print(f"SKELETON {a.package_name}  bones={len(bones)}")
            print("   not in Manny:", only_here[:40])
            print("   Manny bones missing here:", len(missing), missing[:40])
        elif cls == "PhysicsAsset":
            print(f"PHYSICS {a.package_name}")
        else:
            print(f"ANIMBP {a.package_name}")
