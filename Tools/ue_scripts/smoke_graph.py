"""노드 도우미 동작 확인: 임시 BP에 BeginPlay -> Print String을 만들고 컴파일한 뒤 지운다."""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

PATH = "/Game/Portfolio/Dev/BP_GraphSmokeTest"
if EAL.does_asset_exist(PATH):
    EAL.delete_asset(PATH)

bp = BEL.create_blueprint_asset_with_parent(PATH, unreal.Actor)
g = graph(bp)
begin = G.add_event_node(g, unreal.Actor.static_class(), "ReceiveBeginPlay", 0, 0)
prn = G.add_call_function_node(g, load_cls("/Script/Engine.KismetSystemLibrary"), "PrintString", 300, 0)
link(begin, "then", prn, "execute")
set_default(prn, "InString", "Graph smoke test")
BEL.compile_blueprint(bp)
dump_graph(g, pins=False)
print("pins", G.describe_pins(prn)[:3])
unreal.SystemLibrary.collect_garbage()
EAL.delete_asset(PATH)
EAL.delete_directory("/Game/Portfolio/Dev")
print("smoke ok")
