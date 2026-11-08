"""8-5d 절뚝임 걸음: 적 애님 BP의 걷기 블렌드 스페이스 플레이어 Play Rate ← LocomotionPlayRate(UEnemyAnimInstance).

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/wire_limp_play_rate.py [애님 BP ...]
기본: ABP_Enemy_UE4(Wasteland), ABP_EnemyMannequin(그레이박스). Play Rate 핀은 기본으로 숨겨져 있어 ShowPinForProperties로 드러낸다.
이미 연결돼 있으면 건너뛴다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

ABPS = sys.argv[1:] or ["/Game/Showcase/Enemy/ABP_Enemy_UE4", "/Game/Portfolio/Enemies/ABP_EnemyMannequin"]
ENTRY = 'PropertyName="PlayRate"'

for path in ABPS:
    abp = load(path)
    ag = graph(abp, "AnimGraph")
    bs = find_node(ag, "AnimGraphNode_BlendSpacePlayer")
    pins = G.describe_pins(bs)
    if any("PlayRate" in p and "-> 1" in p for p in pins):
        print(path.split("/")[-1], "already wired")
        continue
    if not any(p.split()[1] == "PlayRate" for p in pins if len(p.split()) > 1):
        text = G.get_property_by_path(bs, "ShowPinForProperties")
        i = text.index(ENTRY)
        j = text.index("bCanToggleVisibility=True)", i)
        text = text[:j] + "bShowPin=True," + text[j:]
        set_path(bs, "ShowPinForProperties", text)
    rate = G.add_variable_node(ag, "LocomotionPlayRate", False, None, -1500, 220)
    link(rate, "LocomotionPlayRate", bs, "PlayRate")
    compile_save(abp)
    print(path.split("/")[-1], [p for p in G.describe_pins(bs) if "PlayRate" in p or "Speed" in p])
