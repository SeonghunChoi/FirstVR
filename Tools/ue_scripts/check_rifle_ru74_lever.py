"""8-2d 확인(시뮬레이트 중): 연발 소총 레버 값과 b_SafetyLever 회전.

    python Tools/ue_remote.py Tools/ue_scripts/check_rifle_ru74_lever.py [safe|single|auto]
상태를 주면 조정간처럼 바꾼 뒤(발사 모드 먼저, 잠금은 나중) 값을 출력한다. 각도가 따라가는 것은 다음 실행에서 본다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from pie import *

gun = find_actor("BP_AutoRifleWeapon_RU74")
if gun is None:
    raise RuntimeError("BP_AutoRifleWeapon_RU74 not found in game world (is simulate running?)")
step = gun.get_component_by_class(load_cls("/Script/FirstVR.SafetyStep"))
trigger = gun.get_component_by_class(load_cls("/Script/FirstVR.TriggerInputComponent"))
mesh = [c for c in gun.get_components_by_class(unreal.SkeletalMeshComponent) if c.get_name() == "RifleMesh"][0]

state = sys.argv[1] if len(sys.argv) > 1 else None
if state in ("single", "auto"):
    trigger.set_fire_mode(unreal.FireMode.SINGLE if state == "single" else unreal.FireMode.AUTO)
    step.set_locked(False)
elif state == "safe":
    step.set_locked(True)

anim = mesh.get_anim_instance()
print(f"locked {step.is_locked()} mode {trigger.get_fire_mode()} | anim bSafetyLocked {anim.get_editor_property('bSafetyLocked')}"
      f" bAutoMode {anim.get_editor_property('bAutoMode')} LeverAngle {anim.get_editor_property('LeverAngle'):.1f}")
print("b_SafetyLever component rotation", mesh.get_socket_transform("b_SafetyLever", unreal.RelativeTransformSpace.RTS_COMPONENT).rotation.rotator())
