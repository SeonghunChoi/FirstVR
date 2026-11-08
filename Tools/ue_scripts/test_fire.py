"""9-2e: 시뮬레이트에서 무기를 손 없이 쏴 본다(파지 단계를 '잡힘'으로, 안전장치 해제). 연출 확인용.

    python Tools/ue_remote.py Tools/ue_scripts/test_fire.py <무기 클래스 이름 일부> [aim=<x>,<y>,<z>] [shot=<캡처 이름>] [cam=<x>,<y>,<z>]

aim을 주면 무기를 그 점(적 가슴 등)으로 향하게 옮겨 놓고 쏜다(Muzzle 기준). shot이면 쏜 직후 시간을 늦추고 cam에서 aim을 본 화면을 찍는다.
"""
import sys
import unreal

U = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
w = U.get_game_world()


def main():
    part = sys.argv[1]
    opts = dict(a.split("=", 1) for a in sys.argv[2:] if "=" in a)
    weapon = [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor) if part in a.get_class().get_name()][0]
    for c in weapon.get_components_by_class(unreal.ActorComponent):
        n = c.get_class().get_name()
        if "GripStep" in n and "TwoHand" not in n:
            c.set_held(True)
        if "SafetyStep" in n:
            c.set_locked(False)
    muzzle = [c for c in weapon.get_components_by_class(unreal.SceneComponent) if c.get_name() == "Muzzle"][0]
    if "aim" in opts:
        aim = unreal.Vector(*map(float, opts["aim"].split(",")))
        # 물리를 끄고 총구가 aim을 보도록 액터를 돌린다(총구 방향 = 액터 기준 Muzzle 방향과 같다고 보고 몸통 위치를 aim 쪽 1.5m 앞에)
        for p in weapon.get_components_by_class(unreal.PrimitiveComponent):
            p.set_simulate_physics(False)
        fwd_world = muzzle.get_forward_vector()
        target_dir = (aim - muzzle.get_world_location()); target_dir.normalize()
        delta = unreal.MathLibrary.find_look_at_rotation(unreal.Vector(0, 0, 0), target_dir)
        cur = unreal.MathLibrary.find_look_at_rotation(unreal.Vector(0, 0, 0), fwd_world)
        new_rot = unreal.MathLibrary.compose_rotators(weapon.get_actor_rotation(), unreal.MathLibrary.normalized_delta_rotator(delta, cur))
        weapon.set_actor_rotation(new_rot, True)
        offset = muzzle.get_world_location() - weapon.get_actor_location()
        weapon.set_actor_location(aim - target_dir * 150.0 - offset, False, True)
    weapon.request_fire()
    print(weapon.get_name(), "fired from", muzzle.get_world_location(), "dir", muzzle.get_forward_vector())
    if "shot" in opts:
        unreal.SystemLibrary.execute_console_command(w, "slomo 0.05")
        cam = unreal.Vector(*map(float, opts["cam"].split(",")))
        aim = unreal.Vector(*map(float, opts["aim"].split(","))) if "aim" in opts else muzzle.get_world_location()
        U.set_level_viewport_camera_info(cam, unreal.MathLibrary.find_look_at_rotation(cam, aim))
        unreal.AutomationLibrary.take_high_res_screenshot(1600, 900, f"D:/Projects/Unreal/FirstVR/Saved/Captures/{opts['shot']}.png", delay=0.02, force_game_view=True)


main()
