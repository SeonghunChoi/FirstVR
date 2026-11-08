"""8-5d 절뚝임 걸음: 걷기·조깅 애니메이션에 발이 앞으로 나가는 구간 커브 FootSwing_L / FootSwing_R(0~1)를 구워 넣는다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/bake_foot_swing_curves.py [폴더 또는 시퀀스 경로 ...]
기본: Wasteland 리타깃(Showcase/Enemy/Anims)과 템플릿 마네킹(Characters/Mannequins/Anims/Unarmed/Walk·Jog)의 MF_Unarmed_Walk_*·Jog_*.

UEnemyAnimInstance가 다친 다리 쪽 커브를 읽어, 그 발이 앞으로 나가는 동안 걸음을 더 느리게 한다(LimpSwingPlayRate).
판단: 루트 기준 발 속도를 애니메이션 진행 방향(루트 모션 방향)으로 잰다. 디딘 발은 루트에 대해 뒤로(약 -진행 속도),
앞으로 나가는 발은 앞으로 움직인다. 앞으로 가는 속도가 진행 속도의 30%에 이르면 1(그 사이는 부드럽게).
이미 있으면 지우고 다시 굽는다. 루트 모션이 거의 없는 애니메이션(대기)은 건너뛴다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

APE = unreal.AnimPoseExtensions
AL = unreal.AnimationLibrary
WORLD = unreal.AnimPoseSpaces.WORLD
CURVES = {"l": "FootSwing_L", "r": "FootSwing_R"}
DEFAULT = ["/Game/Showcase/Enemy/Anims", "/Game/Characters/Mannequins/Anims/Unarmed/Walk", "/Game/Characters/Mannequins/Anims/Unarmed/Jog"]


def sequences(args):
    out = []
    for a in args or DEFAULT:
        if EAL.does_directory_exist(a):
            for p in EAL.list_assets(a, recursive=False):
                d = EAL.find_asset_data(p)
                name = str(d.asset_name)
                if d.asset_class_path.asset_name == "AnimSequence" and ("_Walk_" in name or "_Jog_" in name):
                    out.append(p.split(".")[0])
        else:
            out.append(a)
    return out


def smooth(x):
    x = max(0.0, min(1.0, x))
    return x * x * (3 - 2 * x)


opts = unreal.AnimPoseEvaluationOptions()
for path in sequences(sys.argv[1:]):
    seq = load(path)
    keys = AL.get_num_frames(seq) + 1
    length = AL.get_sequence_length(seq)
    dt = length / (keys - 1)
    poses = [APE.get_anim_pose_at_frame(seq, k, opts) for k in range(keys)]
    root = [APE.get_bone_pose(p, "root", WORLD).translation for p in poses]
    travel = root[-1] - root[0]
    travel.z = 0
    speed = travel.length() / length
    if speed < 20:
        print(f"skip {path.split('/')[-1]} (root motion {speed:.0f} cm/s)")
        continue
    direction = travel / travel.length()
    times = [k * dt for k in range(keys)]
    summary = []
    for side, curve in CURVES.items():
        rel = [APE.get_bone_pose(p, f"foot_{side}", WORLD).translation - r for p, r in zip(poses, root)]
        values = []
        for k in range(keys):
            a, b = (rel[k], rel[k + 1]) if k + 1 < keys else (rel[k - 1], rel[k])   # 끝 키는 앞 구간 속도
            forward = (b - a).dot(direction) / dt
            values.append(smooth(forward / (0.3 * speed)))
        if AL.does_curve_exist(seq, curve, unreal.RawCurveTrackTypes.RCT_FLOAT):
            AL.remove_curve(seq, curve, False)
        AL.add_curve(seq, curve, unreal.RawCurveTrackTypes.RCT_FLOAT, False)
        AL.add_float_curve_keys(seq, curve, times, values)
        summary.append(f"{curve} swing {sum(1 for v in values if v > 0.5) / keys:.0%}")
    EAL.save_loaded_asset(seq, only_if_is_dirty=False)
    print(f"{path.split('/')[-1]} speed {speed:.0f} keys {keys} | " + " | ".join(summary))
