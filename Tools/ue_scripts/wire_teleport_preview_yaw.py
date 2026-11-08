"""8-6e: 순간이동 표시의 플레이 영역 사각형 방향을 실제 순간이동 결과와 맞춘다(BP_TeleportVisualizer).

    python Tools/ue_remote.py Tools/ue_scripts/wire_teleport_preview_yaw.py

템플릿은 Tick에서 액터(내릴 자리에 놓임)를 지금 폰 방향(Get Player Pawn → Get Actor Rotation)으로 돌리고
NS_PlayAreaBounds를 머리 거리만큼 반대로 옮겨, 방(가디언)이 이동 뒤 놓일 자리를 그린다.
구간 게임 모드는 내릴 때 몸을 정한 방향으로 돌리므로, 회전 입력을
Make Rotator(Yaw ← FirstVRGameMode::GetTeleportPreviewYaw(폰, 자기 위치))로 바꾼다(정한 방향이 없으면 지금 폰 방향 그대로).
이미 바꿨으면 멈춘다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

bp = load("/Game/XRFramework/Blueprints/BP_TeleportVisualizer")
g = graph(bp)
if find_node(g, "Get Teleport Preview Yaw"):
    raise RuntimeError("already wired")
set_rot = find_node(g, "Set Actor Rotation")
pawn = find_node(g, "Get Player Pawn [K2Node_CallFunction] @(-1413")

G.break_pin_links(set_rot, "NewRotation")
gm_cls = load_cls("/Script/FirstVR.FirstVRGameMode")
yaw = G.add_call_function_node(g, gm_cls, "GetTeleportPreviewYaw", -1380, -5420)
loc = G.add_call_function_node(g, load_cls("/Script/Engine.Actor"), "K2_GetActorLocation", -1640, -5380)
rot = G.add_call_function_node(g, load_cls("/Script/Engine.KismetMathLibrary"), "MakeRotator", -1120, -5440)

link(pawn, "ReturnValue", yaw, "PlayerPawn")
link(loc, "ReturnValue", yaw, "AimedLocation")
link(yaw, "ReturnValue", rot, "Yaw")
link(rot, "ReturnValue", set_rot, "NewRotation")
compile_save(bp)
for n in (yaw, rot, set_rot):
    print(G.describe_node(n)); [print("   ", p) for p in G.describe_pins(n)]
