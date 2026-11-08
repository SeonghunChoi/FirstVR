"""8-5d 다리 접힘 바로잡기: 리타깃한 애니메이션의 IK 발 본 트랙을 UE5 원본 규칙으로 다시 쓴다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/fix_retarget_ik_bones.py [시퀀스 경로 ...]   # 없으면 Showcase/Enemy/Anims 전부

IK 리타기터는 IK 본을 옮기지 않아서, 리타깃 결과는 ik_foot_root가 루트 모션만큼 거꾸로 움직이고(루트를 잠그면 몸 뒤로 멀어짐)
ik_foot_l/r은 레퍼런스 포즈 자리에 멈춰 있다. 그러면 Stride Warping과 Leg IK가 몸 뒤의 목표로 발을 끌어당겨
VR Preview에서 두 다리가 함께 뒤로 접혀 올라갔다. (애님 그래프의 Copy Bone으로 맞추려 했지만 런타임에 반영되지 않았다.)
UE5 원본처럼 ik_foot_root는 root에 붙이고(로컬 0), ik_foot_l/r은 매 키 foot_l/r의 컴포넌트 트랜스폼을 root 기준으로 넣는다.
다른 본 트랙은 건드리지 않는다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

APE = unreal.AnimPoseExtensions
WORLD = unreal.AnimPoseSpaces.WORLD   # AnimPose의 WORLD = 컴포넌트 공간
OUT = "/Game/Showcase/Enemy/Anims"

paths = sys.argv[1:] or [a.split(".")[0] for a in EAL.list_assets(OUT, recursive=False)
                         if EAL.find_asset_data(a).asset_class_path.asset_name == "AnimSequence"]
opts = unreal.AnimPoseEvaluationOptions()

for path in paths:
    seq = load(path)
    keys = unreal.AnimationLibrary.get_num_frames(seq) + 1   # 키 수 = 프레임 수 + 1(끝 키 포함)
    root_keys, foot = [], {"l": [], "r": []}
    for k in range(keys):
        pose = APE.get_anim_pose_at_frame(seq, k, opts)
        root_cs = APE.get_bone_pose(pose, "root", WORLD)
        for side in ("l", "r"):
            local = APE.get_bone_pose(pose, f"foot_{side}", WORLD) * root_cs.inverse()   # 부모(ik_foot_root = root) 기준
            foot[side].append(local)
    ctrl = seq.get_editor_property("controller")
    ctrl.open_bracket(unreal.Text("Fix IK foot bones"))
    ok = [ctrl.set_bone_track_keys("ik_foot_root", [unreal.Vector()] * keys, [unreal.Quat()] * keys, [unreal.Vector(1, 1, 1)] * keys)]
    for side in ("l", "r"):
        ts = foot[side]
        ok.append(ctrl.set_bone_track_keys(f"ik_foot_{side}", [t.translation for t in ts], [t.rotation for t in ts], [t.scale3d for t in ts]))
    ctrl.close_bracket()
    if not all(ok):
        raise RuntimeError(f"{path}: set_bone_track_keys failed {ok} (keys {keys})")
    EAL.save_loaded_asset(seq, only_if_is_dirty=False)

    # 확인: 중간 키에서 ik_foot이 foot과 같은 자리, ik_foot_root가 root와 같은 자리
    pose = APE.get_anim_pose_at_frame(seq, keys // 2, opts)
    g = lambda b: APE.get_bone_pose(pose, b, WORLD).translation
    err = max((g("ik_foot_l") - g("foot_l")).length(), (g("ik_foot_r") - g("foot_r")).length(), (g("ik_foot_root") - g("root")).length())
    print(f"{path.split('/')[-1]} keys {keys} max error {err:.3f} cm")
