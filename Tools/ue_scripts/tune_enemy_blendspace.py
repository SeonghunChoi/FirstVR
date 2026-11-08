"""8-5d 발 미끄러짐: 적 걷기 블렌드 스페이스 사본의 걷기 샘플을 속도 300 → 75로 옮긴다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/tune_enemy_blendspace.py [사본 경로] [원본 경로]
기본은 Wasteland용 BS_Idle_Walk_Run_UE4. 원본 경로를 주면 사본이 없을 때 원본을 복제해 만든다
(그레이박스 마네킹: Portfolio/Enemies/BS_EnemyWalk ← 템플릿 Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run).

적은 150(절뚝이면 75)으로 걷는데 걷기 샘플이 300에 있으면 대기와 반씩 섞인다. 섞인 포즈의 루트 모션 속도도 150이 되어
Stride Warping이 "이미 맞다"고 보고 보폭을 줄이지 않는다. 75 이상에서 걷기만 나오게 하고, 보폭은 Stride Warping이 줄인다.
속도축 격자를 4 → 8칸(75 간격)으로 바꿔야 75에 놓을 수 있다(격자 맞춤이 켜져 있음). 원본(템플릿) 블렌드 스페이스는 그대로.
"""
import re
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

BS = sys.argv[1] if len(sys.argv) > 1 else "/Game/Showcase/Enemy/Anims/BS_Idle_Walk_Run_UE4"
SOURCE = sys.argv[2] if len(sys.argv) > 2 else None
WALK_FROM, WALK_TO = 300, 75

if SOURCE and not EAL.does_asset_exist(BS):
    EAL.duplicate_asset(SOURCE, BS)
    print("duplicated", SOURCE, "->", BS)
bs = load(BS)
params = list(bs.get_editor_property("blend_parameters"))
params[1].set_editor_property("grid_num", 8)
bs.set_editor_property("blend_parameters", params)

text = G.get_property_by_path(bs, "SampleData")
count = 0


def move(m):
    global count
    if "_Walk_" not in m.group(0):
        return m.group(0)
    count += 1
    return m.group(0).replace(f"Y={WALK_FROM:.6f}", f"Y={WALK_TO:.6f}")


new = re.sub(r"\(Animation=[^)]*\)?,SampleValue=\([^)]*\)\)", move, text)
if count != 9:
    raise RuntimeError(f"expected 9 walk samples, moved {count}")
set_path(bs, "SampleData", new)
# 샘플 위치를 속성으로 바꾸면 실행용 블렌드 데이터(삼각 분할)는 그대로라 게임은 옛 위치(300)로 섞는다. 다시 만든다.
if not G.resample_blend_space(bs):
    raise RuntimeError("resample failed")
EAL.save_loaded_asset(bs, only_if_is_dirty=False)

print("speed axis", [(p.get_editor_property("display_name"), p.get_editor_property("grid_num")) for p in bs.get_editor_property("blend_parameters")][:2])
for m in re.finditer(r"/(\w+)\.\w+'\",SampleValue=\(X=([-\d.]+),Y=([-\d.]+)", G.get_property_by_path(bs, "SampleData")):
    print(f"  {m.group(1)}  dir {float(m.group(2)):.0f}  speed {float(m.group(3)):.0f}")
