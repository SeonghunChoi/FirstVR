"""무기를 손에서 놓으면 물리(중력)로 떨어지게 한다. 처음 배치 상태(테이블 위 고정)는 그대로 둔다.

    python Tools/ue_remote.py Tools/ue_scripts/set_weapon_drop_physics.py

템플릿 BP_GrabComponent는 BeginPlay에서 부모가 물리 시뮬레이션 중일 때만 bSimulateOnDrop을 켜고(끄지는 않음),
놓을 때 이 값이 참이면 부모(Body)의 물리를 켠다. 무기 Body는 처음에 물리가 꺼져 있어 놓으면 공중에 멈췄다.
→ 그레이박스 무기 BP의 잡기 컴포넌트 Grip 기본값 bSimulateOnDrop = True. 꾸민 무기(자식)는 물려받는다.
보조 손잡이(SupportGrip, Custom)는 붙이지 않는 잡기라 해당 없음(보조 손 들기는 UTwoHandGripStep이 직접 물리를 켬).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary

GRAYBOX = ["/Game/Portfolio/Weapons/BP_PistolWeapon", "/Game/Portfolio/Weapons/BP_RifleWeapon",
           "/Game/Portfolio/Weapons/BP_AutoRifleWeapon", "/Game/Portfolio/Weapons/BP_AssembledRifle",
           "/Game/Portfolio/Weapons/BP_TestWeapon"]
CHILDREN = ["/Game/Showcase/Weapon/BP_PistolWeapon_PM", "/Game/Showcase/Weapon/BP_RifleWeapon_AR",
            "/Game/Showcase/Weapon/BP_AutoRifleWeapon_RU74"]


def grip_template(bp):
    for h in sds.k2_gather_subobject_data_for_blueprint(bp):
        data = lib.get_data(h)
        if str(lib.get_variable_name(data)) == "Grip":
            return lib.get_object_for_blueprint(data, bp)
    return None


for path in GRAYBOX:
    bp = load(path)
    t = grip_template(bp)
    if t is None:
        print(path.split("/")[-1], "no Grip component")
        continue
    t.modify()
    t.set_editor_property("bSimulateOnDrop", True)
    compile_save(bp)
    print(path.split("/")[-1], "Grip bSimulateOnDrop", t.get_editor_property("bSimulateOnDrop"))

# 자식이 값을 덮어쓰고 있지 않은지 확인만 한다
for path in CHILDREN:
    bp = load(path)
    t = grip_template(bp)
    print(path.split("/")[-1], "Grip bSimulateOnDrop", t.get_editor_property("bSimulateOnDrop") if t else None)
