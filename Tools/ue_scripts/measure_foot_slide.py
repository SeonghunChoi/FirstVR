"""8-5d 조사: 시뮬레이트 중 걷는 적의 발이 월드에서 얼마나 미끄러지는지 잰다. 변경 없음.

    python Tools/ue_remote.py Tools/ue_scripts/measure_foot_slide.py start [초]   # 에디터 틱마다 기록 시작(기본 3초)
    python Tools/ue_remote.py Tools/ue_scripts/measure_foot_slide.py report       # 결과

디딤발(액터 기준 가장 낮은 높이 + 3cm 안, 걷는 동안)의 월드 수평 속도가 0에 가까우면 미끄러지지 않는다. 몸 속도와 같이 출력한다.
기록은 builtins에 둔다(ue_remote가 모듈 캐시를 비우므로).
"""
import builtins
import statistics
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
cmd = sys.argv[1]

if cmd == "start":
    duration = float(sys.argv[2]) if len(sys.argv) > 2 else 3.0
    state = {"rows": {}, "handle": None}
    builtins._foot_slide = state

    def tick(dt):
        w = UES.get_game_world()
        if not w:
            return
        now = unreal.GameplayStatics.get_time_seconds(w)
        state.setdefault("t0", now)
        if now - state["t0"] > duration:
            unreal.unregister_slate_post_tick_callback(state["handle"])
            state["handle"] = None
            return
        for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Character):
            if "Enemy" not in a.get_class().get_name():
                continue
            m = a.get_editor_property("mesh")
            if a.get_name() not in state["rows"]:
                # 시뮬레이트 화면에 안 보이는 적은 뼈를 갱신하지 않아 발이 몸과 같이 움직인 것처럼 나온다(첫 측정에서 발 = 몸 속도)
                m.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
            v = a.get_velocity()
            fl, fr = m.get_socket_location("foot_l"), m.get_socket_location("foot_r")
            state["rows"].setdefault(a.get_name(), []).append(
                (now, fl, fr, (v.x ** 2 + v.y ** 2) ** 0.5, a.get_actor_transform().inverse_transform_location(fl).z,
                 a.get_actor_transform().inverse_transform_location(fr).z))

    state["handle"] = unreal.register_slate_post_tick_callback(tick)
    print("recording", duration, "s")

elif cmd == "report":
    state = getattr(builtins, "_foot_slide", None)
    if not state:
        raise RuntimeError("no recording")
    print("still recording" if state["handle"] else "done")
    for name, rows in state["rows"].items():
        rows = [r for r in rows if r[3] > 10]   # 걷는 동안만
        if len(rows) < 10:
            continue
        body = statistics.median(r[3] for r in rows)
        parts = [f"{name} frames {len(rows)} body {body:.0f} cm/s"]
        for i, bone in ((1, "foot_l"), (2, "foot_r")):
            low = min(r[3 + i] for r in rows)   # 액터 기준 높이(지형 경사 무시)
            sp = [((b[i].x - a[i].x) ** 2 + (b[i].y - a[i].y) ** 2) ** 0.5 / (b[0] - a[0])
                  for a, b in zip(rows, rows[1:]) if b[0] > a[0] and a[3 + i] < low + 3 and b[3 + i] < low + 3]
            if sp:
                parts.append(f"{bone} stance median {statistics.median(sp):.0f} cm/s ({len(sp)} frames)")
        print(" | ".join(parts))
