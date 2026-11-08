"""9-2e: 꾸민 콘텐츠(Showcase)에 피격 연출을 연결한다. 에셋은 Fab `Realistic_Starter_VFX_Pack_Vol2`(캐스케이드).

    python Tools/ue_remote.py Tools/ue_scripts/wire_hit_effects.py

- 벽·바닥(탄 데이터): 그레이박스 탄 데이터는 git이라 팩을 참조할 수 없다 → Showcase에 같은 이름의 탄 데이터 사본을 만들고
  꾸민 탄창 BP의 AmmoType을 사본으로 바꾼다. 연출은 P_Default(불티와 파편, 한 번 재생).
- 적: Wasteland(살) = P_Blood_Splat_Cone 크게, SciFi Soldier(장갑) = P_Default. 솔저는 Wasteland 적 데이터를 공유했으므로
  데이터 사본 DA_Enemy_SciFi를 만들어 솔저 BP의 DamageReceiver만 바꾼다.
팩의 P_Sparks_A~F는 반복 재생이라 쓰지 않는다(풀링 스폰이 끝나지 않음). 다시 실행해도 된다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()
EAL = unreal.EditorAssetLibrary
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary
FX = "/Game/Realistic_Starter_VFX_Pack_Vol2/Particles/"
WALL = load(FX + "Hit/P_Default")
BLOOD = load(FX + "Blood/P_Blood_Splat_Cone")
DATA_DIR = "/Game/Showcase/Weapon/Data"


def copy(src, dst):
    if EAL.does_asset_exist(dst):
        return load(dst)
    return EAL.duplicate_asset(src, dst)


# --- 탄 데이터 사본 + 꾸민 탄창
ammo = {}
for n in ("DA_Ammo_9mm", "DA_Ammo_556"):
    a = copy(f"/Game/Portfolio/Data/{n}", f"{DATA_DIR}/{n}")
    a.set_editor_property("impact_effect", WALL)
    a.set_editor_property("impact_effect_scale", unreal.Vector(0.2, 0.2, 0.2))   # 1.0·0.4는 너무 멀리 튐(VR)
    EAL.save_loaded_asset(a, only_if_is_dirty=False)
    ammo[n] = a
    print("ammo", a.get_path_name(), "impact", a.get_editor_property("impact_effect").get_name())

for path, n in (("/Game/Showcase/Weapon/BP_Magazine_AR", "DA_Ammo_556"), ("/Game/Showcase/Weapon/BP_Magazine_RU74", "DA_Ammo_556"),
                ("/Game/Showcase/Weapon/BP_Magazine_PM", "DA_Ammo_9mm")):
    bp = load(path)
    cdo = unreal.get_default_object(bp_class(bp))
    cdo.modify()
    cdo.set_editor_property("AmmoType", ammo[n])
    compile_save(bp)
    print(path.split("/")[-1], "AmmoType", unreal.get_default_object(bp_class(bp)).get_editor_property("AmmoType").get_path_name())

# --- 적 데이터
wl = load("/Game/Showcase/Enemy/DA_Enemy_Wasteland")
wl.set_editor_property("hit_effect", BLOOD)
wl.set_editor_property("hit_effect_scale", unreal.Vector(0.6, 0.6, 0.6))   # 2.5·1.0은 너무 멀리 튐(VR, 2026-11-09)
EAL.save_loaded_asset(wl, only_if_is_dirty=False)

sci = copy("/Game/Showcase/Enemy/DA_Enemy_Wasteland", "/Game/Showcase/Enemy/DA_Enemy_SciFi")
sci.set_editor_property("hit_effect", WALL)
sci.set_editor_property("hit_effect_scale", unreal.Vector(0.25, 0.25, 0.25))
EAL.save_loaded_asset(sci, only_if_is_dirty=False)

bp = load("/Game/Showcase/Enemy/BP_EnemySciFiSoldier")
for h in sds.k2_gather_subobject_data_for_blueprint(bp):
    data = lib.get_data(h)
    if str(lib.get_variable_name(data)) == "DamageReceiver":
        t = lib.get_object_for_blueprint(data, bp)
        t.modify()
        t.set_editor_property("enemy_data", sci)
compile_save(bp)
cdo = unreal.get_default_object(bp_class(bp))
print("scifi data", cdo.get_component_by_class(load_cls("/Script/FirstVR.DamageReceiver")).get_editor_property("enemy_data").get_name())
print("wasteland hit", wl.get_editor_property("hit_effect").get_name())
