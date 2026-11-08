"""9-2b S11: 적 공격 몽타주의 재생 위치를 보고, 정한 구간(전체 길이 대비 비율)에 들어오면 시간을 거의 멈춘다(slomo 0.0001).
밖에서 반복 호출한다(게임 스레드를 막지 않게 한 번에 한 번만 본다).

    python Tools/ue_remote.py Tools/ue_scripts/attack_freeze.py <시작 비율> <끝 비율> [slomo=0.2] [near=300] [shot=<VR 사진 이름>]
출력: 적마다 "몽타주 위치/길이", 멈추면 FROZEN <적 번호>
"""
import sys
import unreal

w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
lo, hi = float(sys.argv[1]), float(sys.argv[2])
opts = dict(a.split("=", 1) for a in sys.argv[3:] if "=" in a)
enemies = sorted([a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Character) if "Enemy" in a.get_class().get_name()],
                 key=lambda a: a.get_name())
frozen = None
for i, e in enumerate(enemies):
    inst = e.get_component_by_class(unreal.SkeletalMeshComponent).get_anim_instance()
    m = inst.get_current_active_montage()
    if not m:
        print(i, e.get_name(), "no montage"); continue
    pos, length = inst.montage_get_position(m), m.get_play_length()
    print(i, e.get_name(), m.get_name(), round(pos, 2), "/", round(length, 2))
    if frozen is None and lo <= pos / length <= hi:
        frozen = i
if frozen is not None:
    unreal.SystemLibrary.execute_console_command(w, "slomo 0.0001")
    print("FROZEN", frozen)
    if "shot" in opts:   # 멈춘 같은 틱에 관전자 화면을 찍는다 (vr_shot.py와 같은 경로)
        out = "D:/Projects/Unreal/FirstVR/Docs/Portfolio/Shots/vr"
        unreal.SystemLibrary.execute_console_command(w, f"HighResShot 1 filename={out}/{opts['shot']}.png")
        print("shot ->", opts["shot"])
elif "slomo" in opts:
    # 적이 플레이어 가까이(near cm, 기본 300) 왔을 때만 늦춘다. 멀리 있을 때 늦추면 웨이브 전체가 느려진다.
    pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
    near = float(opts.get("near", "300"))
    cam = pawn.get_component_by_class(unreal.CameraComponent) if pawn else None   # VR 폰 원점이 아니라 머리
    head = cam.get_world_location() if cam else None
    dists = [round((e.get_actor_location() - head).length()) for e in enemies] if head else []
    print("distance to head", dists)
    close = any(d <= near for d in dists)
    unreal.SystemLibrary.execute_console_command(w, f"slomo {opts['slomo'] if close else 1}")
    print("slomo", opts["slomo"] if close else 1)
