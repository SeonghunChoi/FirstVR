"""8-5d 조사: 걷기 애니메이션의 디딤발 속도(발이 바닥에 붙어 뒤로 가는 속도 = 미끄러지지 않을 이동 속도)를 잰다. 변경 없음.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/measure_stride.py <시퀀스 경로> [...]

컴포넌트 공간 foot_l/foot_r 위치를 1/120초 간격으로 샘플링해, 발이 가장 낮은 구간(최저 높이 + 3cm 안)의 수평 속도 중앙값을 낸다.
"""
import statistics
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

APE = unreal.AnimPoseExtensions
opts = unreal.AnimPoseEvaluationOptions()
DT = 1.0 / 120

for path in sys.argv[1:]:
    seq = load(path)
    length = unreal.AnimationLibrary.get_sequence_length(seq)
    out = []
    for bone in ("foot_l", "foot_r"):
        pts = []
        t = 0.0
        while t <= length:
            pose = APE.get_anim_pose_at_time(seq, t, opts)
            pts.append(APE.get_bone_pose(pose, bone, unreal.AnimPoseSpaces.WORLD).translation)
            t += DT
        low = min(p.z for p in pts)
        speeds = [((b.x - a.x) ** 2 + (b.y - a.y) ** 2) ** 0.5 / DT
                  for a, b in zip(pts, pts[1:]) if a.z < low + 3 and b.z < low + 3]
        out.append(f"{bone} stance {statistics.median(speeds):.0f} cm/s ({len(speeds)} samples, low z {low:.1f})")
    root = APE.get_bone_pose(APE.get_anim_pose_at_time(seq, length, opts), "root", unreal.AnimPoseSpaces.WORLD).translation
    print(path.split("/")[-1], f"length {length:.2f}s", "|", " | ".join(out), "| root end", root)
