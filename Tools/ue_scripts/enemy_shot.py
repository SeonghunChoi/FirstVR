"""9-2b S8~S11: 시뮬레이트 중인 적을 기준으로 카메라를 놓고 반응을 일으킨 뒤 찍는다.

    python Tools/ue_remote.py Tools/ue_scripts/enemy_shot.py <이름|-> <동작> [옵션...]
      동작: none | hit <본> <세기 cm/s> | limp <Left|Right> | kill | halt
      옵션: place=<x>,<y>,<yaw> idx=<적 번호, 0부터> side=<적 정면 기준 카메라 각도, 0=정면> dist=<cm> h=<카메라 높이, 적 발 기준>
            look=<바라볼 높이, 적 발 기준> slomo=<배속> delay=<찍기까지 실제 초> size=<가로>x<세로>

결과는 Docs/Portfolio/Shots/<이름>.png. halt는 그 적의 판단(State Tree)을 멈춘다(제자리에 세워 두고 찍기).
"""
import math
import os
import sys
import unreal

UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
OUT = r"D:\Projects\Unreal\FirstVR\Docs\Portfolio\Shots"


def main():
    args = sys.argv[1:]
    opts = dict(a.split("=", 1) for a in args if "=" in a)
    args = [a for a in args if "=" not in a]
    name, action = args[0], args[1]
    w = UES.get_game_world()
    enemies = [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Character) if "Enemy" in a.get_class().get_name()]
    enemies.sort(key=lambda a: a.get_name())
    e = enemies[int(opts.get("idx", "0"))]
    if "place" in opts:   # place=x,y,yaw: 그 자리로 옮겨 세운다(높이는 지금 그대로)
        px, py, pyaw = map(float, opts["place"].split(","))
        e.set_actor_location_and_rotation(unreal.Vector(px, py, e.get_actor_location().z), unreal.Rotator(0, 0, pyaw), False, True)
    feet = e.get_actor_location() - unreal.Vector(0, 0, e.get_component_by_class(unreal.CapsuleComponent).get_scaled_capsule_half_height())

    if "slomo" in opts:
        unreal.SystemLibrary.execute_console_command(w, f"slomo {opts['slomo']}")

    if action == "hit":
        bone, power = args[2], float(args[3])
        fwd = e.get_actor_forward_vector()
        e.get_component_by_class(unreal.HitReaction).apply_physical_hit(bone, fwd * -power)   # 정면에서 맞아 뒤로
    elif action == "limp":
        e.get_component_by_class(unreal.DamageReceiver).start_limp(unreal.LegSide.LEFT if args[2] == "Left" else unreal.LegSide.RIGHT)
    elif action == "kill":
        e.get_component_by_class(unreal.DamageReceiver).kill()
    elif action == "halt":
        c = e.get_controller()
        c.get_component_by_class(unreal.BrainComponent).stop_logic("photo")
        c.stop_movement()

    # 카메라: 적 정면(Yaw)에서 side도 돌린 방향으로 dist만큼, 발 기준 높이 h
    yaw = e.get_actor_rotation().yaw + float(opts.get("side", "0"))
    d = float(opts.get("dist", "300"))
    cam = feet + unreal.Vector(math.cos(math.radians(yaw)) * d, math.sin(math.radians(yaw)) * d, float(opts.get("h", "150")))
    tgt = feet + unreal.Vector(0, 0, float(opts.get("look", "100")))
    UES.set_level_viewport_camera_info(cam, unreal.MathLibrary.find_look_at_rotation(cam, tgt))
    print(e.get_name(), "feet", feet, "action", action)

    if name != "-":
        W, H = map(int, opts.get("size", "2560x1440").split("x"))
        path = os.path.join(OUT, name + ".png")
        unreal.AutomationLibrary.take_high_res_screenshot(W, H, path, delay=float(opts.get("delay", "0.5")), force_game_view=True)
        print("shot ->", path)


main()
