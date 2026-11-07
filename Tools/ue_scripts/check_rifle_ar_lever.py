"""8-2d 확인(시뮬레이트 중): 단발 소총 레버 값과 b_Safety 회전.

    python Tools/ue_remote.py Tools/ue_scripts/check_rifle_ar_lever.py [toggle]
toggle을 주면 안전장치를 한 번 바꾼 뒤 값을 출력한다(값이 따라가는 것은 다음 실행에서 본다).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from pie import *

gun = find_actor("BP_RifleWeapon_AR")
if gun is None:
    raise RuntimeError("BP_RifleWeapon_AR not found in game world (is simulate running?)")
step = gun.get_component_by_class(load_cls("/Script/FirstVR.SafetyStep"))
mesh = [c for c in gun.get_components_by_class(unreal.SkeletalMeshComponent) if c.get_name() == "RifleMesh"][0]
if len(sys.argv) > 1 and sys.argv[1] == "toggle":
    step.toggle_safety()
anim = mesh.get_anim_instance()
print("locked", step.is_locked(), "| anim", anim.get_class().get_name() if anim else None,
      "| bSafetyLocked", anim.get_editor_property("bSafetyLocked"), "LeverAngle", round(anim.get_editor_property("LeverAngle"), 1))
t = mesh.get_socket_transform("b_Safety", unreal.RelativeTransformSpace.RTS_COMPONENT)
print("b_Safety component rotation", t.rotation.rotator())
