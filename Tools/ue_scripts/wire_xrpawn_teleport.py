"""8-6c: 템플릿 순간이동을 게임 모드에 묻게 바꾼다(BP_XRPawn).

    python Tools/ue_remote.py Tools/ue_scripts/wire_xrpawn_teleport.py

IsValidTeleportLocation
  내비 투영(템플릿) 뒤 Get Game Mode → Cast To FirstVRGameMode
  성공: CanTeleportTo(내비 점) → 새 반환 노드(자리 = Destination, 결과 = 내비 성공 AND 허용)
  실패: 원래 반환 노드(템플릿 그대로, 다른 게임 모드의 레벨)
TryTeleport
  Set bValidTeleportLocation 뒤 Get Game Mode → Cast To FirstVRGameMode
  성공: TeleportPlayerTo(self, ProjectedTeleportLocation)   실패: 원래 Teleport 노드
이미 바꿨으면 멈춘다. 망치면 reload_packages로 디스크 상태로 되돌리고 다시.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

PAWN = "/Game/XRFramework/Blueprints/BP_XRPawn"
bp = load(PAWN)
gm_cls = load_cls("/Script/FirstVR.FirstVRGameMode")
gs_cls = load_cls("/Script/Engine.GameplayStatics")
math_cls = load_cls("/Script/Engine.KismetMathLibrary")
result_cls = load_cls("/Script/BlueprintGraph.K2Node_FunctionResult")
self_cls = load_cls("/Script/BlueprintGraph.K2Node_Self")


def cast_out_pin(node):
    for p in G.describe_pins(node):
        name = p[4:].split(" (")[0]   # "out  AsFirst VRGame Mode (...) : ..."
        if p.startswith("out") and name.startswith("As"):
            return name
    raise RuntimeError("cast output pin not found: " + "\n".join(G.describe_pins(node)))


def need(g, title):
    n = find_node(g, title)
    if n is None:
        raise RuntimeError(f"node '{title}' not found in {g.get_name()}")
    return n


def game_mode_cast(g, x, y):
    get_gm = G.add_call_function_node(g, gs_cls, "GetGameMode", x, y + 140)
    cast = G.add_cast_node(g, gm_cls, x + 260, y)
    link(get_gm, "ReturnValue", cast, "Object")
    return cast, cast_out_pin(cast)


# ── IsValidTeleportLocation ──
g = graph(bp, "IsValidTeleportLocation")
if find_node(g, "Can Teleport To"):
    raise RuntimeError("already wired")
entry = need(g, "K2Node_FunctionEntry")
result = need(g, "K2Node_FunctionResult")
proj = need(g, "Project Point to Navigation")

G.break_pin_links(entry, "then")
cast, as_gm = game_mode_cast(g, 3136, 1760)
can = G.add_call_function_node(g, gm_cls, "CanTeleportTo", 3520, 1760)
both = G.add_call_function_node(g, math_cls, "BooleanAND", 3520, 1960)
result2 = G.add_node(g, result_cls, 3900, 1760)
print("result2 pins", G.describe_pins(result2))

link(entry, "then", cast, "execute")
link(cast, "CastFailed", result, "execute")          # 다른 게임 모드: 템플릿 그대로
link(cast, "then", can, "execute")
link(cast, as_gm, can, "self")
link(proj, "ProjectedLocation", can, "AimedLocation")
link(proj, "ReturnValue", both, "A")
link(can, "ReturnValue", both, "B")
link(can, "then", result2, "execute")
link(can, "Destination", result2, "ProjectedLocation")
link(both, "ReturnValue", result2, "ReturnValue")

# ── TryTeleport ──
g2 = graph(bp, "TryTeleport")
set_valid = need(g2, "Set bValidTeleportLocation")
teleport = need(g2, "Teleport [K2Node_CallFunction")
get_proj = need(g2, "Get ProjectedTeleportLocation")

G.break_pin_links(set_valid, "then")
cast2, as_gm2 = game_mode_cast(g2, 2700, -320)
me = G.add_node(g2, self_cls, 3000, -120)
tp = G.add_call_function_node(g2, gm_cls, "TeleportPlayerTo", 3100, -320)

link(set_valid, "then", cast2, "execute")
link(cast2, "CastFailed", teleport, "execute")       # 다른 게임 모드: 템플릿 Teleport
link(cast2, "then", tp, "execute")
link(cast2, as_gm2, tp, "self")
link(me, "self", tp, "PlayerPawn")
link(get_proj, "ProjectedTeleportLocation", tp, "AimedLocation")

compile_save(bp)
for name, gg in (("IsValidTeleportLocation", g), ("TryTeleport", g2)):
    print("=====", name)
    dump_graph(gg, pins=False)
