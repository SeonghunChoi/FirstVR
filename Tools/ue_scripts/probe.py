"""지점들의 바닥 높이(지형·바닥까지 위에서부터 걸리는 것들)와 그 자리에 사람 크기 캡슐이 들어가는지 본다.
    python Tools/ue_remote.py Tools/ue_scripts/probe.py x1,y1 x2,y2 ...
"""
import sys
import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
for arg in sys.argv[1:]:
    x, y = map(float, arg.split(","))
    ign, chain, floor = [], [], None
    for _ in range(8):
        r = unreal.SystemLibrary.line_trace_single(w, unreal.Vector(x, y, 1500), unreal.Vector(x, y, -1000),
                                                   unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, ign, unreal.DrawDebugTrace.NONE, True)
        h = r.to_tuple() if r else (False,)
        if not h[0]:
            break
        chain.append(f"{h[4].z:.0f}:{h[9].get_actor_label()[:22]}")
        floor = h[4].z
        ign.append(h[9])
        if "Landscape" in h[9].get_actor_label():
            break
    # 지형 바로 위 첫 표면(지형보다 30cm 이내면 바닥으로 본다)
    top = None
    for c in chain:
        z = float(c.split(":")[0])
        if floor is not None and z - floor < 30:
            top = z; break
    clear = None
    if top is not None:
        s = unreal.Vector(x, y, top + 100)
        r = unreal.SystemLibrary.capsule_trace_single(w, s, s + unreal.Vector(0, 0, 1), 35, 88,
                                                      unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True)
        clear = not (r and r.to_tuple()[0])
    print(f"({x:.0f},{y:.0f}) floor {top} clear {clear} | {' > '.join(chain)}")
