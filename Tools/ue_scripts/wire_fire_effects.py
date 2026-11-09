"""9-2e: 발사 연출 부품(UFireEffectsComponent)을 무기 BP에 붙이고 에셋을 정한다.

    python Tools/ue_remote.py Tools/ue_scripts/wire_fire_effects.py

- 그레이박스 무기(git): FireEffects 부품 + 총소리만(템플릿 Fire_Cue). 팩 에셋은 참조하지 않는다.
- 꾸민 무기(Showcase): 물려받은 FireEffects를 덮어써 팩 총구 화염·탄피, 소총은 메시 소켓 ShellEjectionSocket에 ShellEject 기준점.
  Makarov는 권총 팩에 파티클이 없어 소총 총구 화염을 작게 쓴다(탄피는 소총 탄피를 작게).
  총소리는 Fab `Free Weapon Sound Effects`(돌격소총·권총 발사음, 소총 둘은 서로 다른 변형).
다시 실행해도 된다(있으면 추가하지 않고 값만 다시 넣음).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary
FX_CLS = load_cls("/Script/FirstVR.FireEffectsComponent")
FIRE_CUE = load("/Game/XRFramework/Audio/Fire_Cue")

GRAYBOX = ["/Game/Portfolio/Weapons/BP_PistolWeapon", "/Game/Portfolio/Weapons/BP_RifleWeapon",
           "/Game/Portfolio/Weapons/BP_AutoRifleWeapon", "/Game/Portfolio/Weapons/BP_AssembledRifle"]
P = "/Game/FPS_Assault_Pack/Particles/Emitters/"
S = "/Game/FreeWeaponSFX/Cue/"   # Fab Free Weapon Sound Effects(SoundLab1), git 제외
SHOWCASE = {
    "/Game/Showcase/Weapon/BP_RifleWeapon_AR": dict(muzzle=P + "Rifle/P_Rifle_Muzzleflash", shell=P + "Rifle/P_Rifle_ShellEject", scale=1.0, socket=True,
                                                     sound=S + "AssaultRifle/Gunshots/sfx_wpn_ar_gunshot_01_Cue"),
    "/Game/Showcase/Weapon/BP_AutoRifleWeapon_RU74": dict(muzzle=P + "AR74/P_RU74_Muzzleflash", shell=P + "AR74/P_RU74_ShellEject", scale=1.0, socket=True,
                                                          sound=S + "AssaultRifle/Gunshots/sfx_wpn_ar_gunshot_02_Cue"),
    "/Game/Showcase/Weapon/BP_PistolWeapon_PM": dict(muzzle=P + "Rifle/P_Rifle_Muzzleflash", shell=None, scale=0.6, socket=False,
                                                     sound=S + "Handgun/Gunshots/sfx_wpn_hgn_gunshot_01_Cue"),
}


def handles(bp):
    out = {}
    for h in sds.k2_gather_subobject_data_for_blueprint(bp):
        out[str(lib.get_variable_name(lib.get_data(h)))] = h
    return out


def template_of(bp, name):
    h = handles(bp).get(name)
    return lib.get_object_for_blueprint(lib.get_data(h), bp) if h else None


def add_component(bp, cls, name, parent_name=None):
    hs = handles(bp)
    if name in hs:
        return hs[name], False
    parent = hs[parent_name] if parent_name else sds.k2_gather_subobject_data_for_blueprint(bp)[0]
    h, fail = sds.add_new_subobject(unreal.AddNewSubobjectParams(parent_handle=parent, new_class=cls, blueprint_context=bp))
    if not str(fail) == "" and not lib.is_handle_valid(h):
        raise RuntimeError(f"add {name} failed: {fail}")
    sds.rename_subobject(h, unreal.Text(name))
    return h, True


for path in GRAYBOX:
    bp = load(path)
    _, added = add_component(bp, FX_CLS, "FireEffects")
    t = template_of(bp, "FireEffects")
    t.modify()
    t.set_editor_property("FireSound", FIRE_CUE)
    compile_save(bp)
    print(path.split("/")[-1], "FireEffects", "added" if added else "exists", "sound", t.get_editor_property("FireSound").get_name())

for path, cfg in SHOWCASE.items():
    bp = load(path)
    if cfg["socket"]:
        h, added = add_component(bp, unreal.SceneComponent, "ShellEject", "RifleMesh")
        # SCS 노드의 부모 소켓: 메시의 탄피 배출 소켓
        # Python에 SCS가 안 보여 리플렉션 경로로 찾는다(FirstVREditor)
        scs_path = G.get_property_by_path(bp, "SimpleConstructionScript").split("'")[1]
        scs = unreal.find_object(None, scs_path)
        for ref in G.get_property_by_path(scs, "AllNodes").strip("()").split(","):
            node = unreal.find_object(None, ref.strip('"').split("'")[1])
            if G.get_property_by_path(node, "InternalVariableName") == "ShellEject":
                G.set_property_by_path(node, "AttachToName", "ShellEjectionSocket")
                print("  ShellEject socket", G.get_property_by_path(node, "AttachToName"), "parent", G.get_property_by_path(node, "ParentComponentOrVariableName"))
    t = template_of(bp, "FireEffects")
    if t is None:
        raise RuntimeError(path + ": FireEffects not inherited (graybox parent not wired?)")
    t.modify()
    t.set_editor_property("MuzzleEffect", load(cfg["muzzle"]))
    s = cfg["scale"]
    t.set_editor_property("MuzzleEffectScale", unreal.Vector(s, s, s))
    if cfg["shell"]:
        t.set_editor_property("ShellEjectEffect", load(cfg["shell"]))
    t.set_editor_property("FireSound", load(cfg["sound"]))
    compile_save(bp)
    print(path.split("/")[-1], "muzzle", t.get_editor_property("MuzzleEffect").get_name(), "shell", t.get_editor_property("ShellEjectEffect"),
          "sound", t.get_editor_property("FireSound").get_name())
