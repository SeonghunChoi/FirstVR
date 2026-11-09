"""9-2e: 꾸민 레벨의 근접 전투 소리를 연결한다(사용자가 시청 페이지에서 고름, 2026-11-09).

    python Tools/ue_remote.py Tools/ue_scripts/wire_combat_sounds.py [whoosh_src=<폴더>] [damage_src=<폴더>]

- 적 휘두르기(UEnemyAttack, 판정 구간 시작) = 적 데이터 AttackSwingSound
  ← Fab `General Whoosh SFX`(Sound Kajiya) `GW_Whoosh_T1_1` → `Content/GeneralWhooshSFX/`
- 적이 총에 맞음 = 적 데이터 HitSound ← Fab `Damage Vocalizations (Male)`(VoiceBosch) 19번(배우 TYR)
- 플레이어 피격(UPlayerHitFeedback.HitSound) ← 같은 팩 02번(배우 REN). ②와 ③은 다른 목소리.
  → `Content/DamageVocalizations/`
두 폴더는 git 제외(.gitignore). 에셋이 없으면 아래 WAV에서 가져온다(팩 zip을 풀어 둔 곳, 다시 가져올 때만 필요).

플레이어 피격음은 git 에셋인 템플릿 BP_XRPawn에 있어 팩을 참조할 수 없다
→ Showcase 자식 폰 BP_XRPawn_Showcase에서 덮어쓰고, 꾸민 게임 모드(공장·연구소)의 DefaultPawnClass를 바꾼다.
다시 실행해도 된다.
"""
import os
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()
EAL = unreal.EditorAssetLibrary
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()

opts = dict(a.split("=", 1) for a in sys.argv[1:] if "=" in a)   # whoosh_src=<폴더> damage_src=<폴더>
WHOOSH_SRC = opts.get("whoosh_src", r"D:\Projects\Unreal\FirstVR\general_whoosh_48khz_16bit\One Shot")
DAMAGE_SRC = opts.get("damage_src", r"D:\Projects\Unreal\FirstVR\Damage Sounds")
SOUNDS = {
    # 이름: (에셋 경로, 원본 WAV)
    "swing": ("/Game/GeneralWhooshSFX/SW_Whoosh_T1_1", os.path.join(WHOOSH_SRC, "GW_Whoosh_T1_1.wav")),
    "player": ("/Game/DamageVocalizations/SW_DamageGrunt_02", os.path.join(DAMAGE_SRC, "02. Damage Grunt (Male).wav")),
    "enemy": ("/Game/DamageVocalizations/SW_DamageGrunt_19", os.path.join(DAMAGE_SRC, "19. Damage Grunt (Male).wav")),
}


def sound(key):
    path, wav = SOUNDS[key]
    if not EAL.does_asset_exist(path):
        if not os.path.exists(wav):
            raise RuntimeError(f"{path} 없음, 원본 WAV도 없음: {wav}")
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", wav)
        t.set_editor_property("destination_path", path.rsplit("/", 1)[0])
        t.set_editor_property("destination_name", path.rsplit("/", 1)[1])
        t.set_editor_property("automated", True)
        t.set_editor_property("save", True)
        t.set_editor_property("replace_existing", True)
        tools.import_asset_tasks([t])
        print("imported", path, "<-", wav)
    return load(path)


SWING, PLAYER_HIT, ENEMY_HIT = sound("swing"), sound("player"), sound("enemy")

# --- 적 데이터
for p in ("/Game/Showcase/Enemy/DA_Enemy_Wasteland", "/Game/Showcase/Enemy/DA_Enemy_SciFi"):
    d = load(p)
    d.set_editor_property("attack_swing_sound", SWING)
    d.set_editor_property("hit_sound", ENEMY_HIT)
    EAL.save_loaded_asset(d, only_if_is_dirty=False)
    print(p.split("/")[-1], "swing", d.get_editor_property("attack_swing_sound").get_name(), "hit", d.get_editor_property("hit_sound").get_name())

# --- 꾸민 폰(템플릿 폰의 자식)
PAWN = "/Game/Showcase/Game/BP_XRPawn_Showcase"
if EAL.does_asset_exist(PAWN):
    pawn = load(PAWN)
else:
    f = unreal.BlueprintFactory()
    f.set_editor_property("parent_class", load("/Game/XRFramework/Blueprints/BP_XRPawn").generated_class())
    pawn = tools.create_asset("BP_XRPawn_Showcase", "/Game/Showcase/Game", unreal.Blueprint, f)
    print("created", pawn.get_path_name())

feedback = None
for h in sds.k2_gather_subobject_data_for_blueprint(pawn):
    data = lib.get_data(h)
    obj = lib.get_object_for_blueprint(data, pawn)
    if obj and obj.get_class().get_name() == "PlayerHitFeedback":
        feedback = obj
if feedback is None:
    raise RuntimeError("PlayerHitFeedback not found on pawn")
feedback.modify()
feedback.set_editor_property("HitSound", PLAYER_HIT)
feedback.set_editor_property("HitSoundPitch", 1.0)
compile_save(pawn)
print("pawn hit sound", feedback.get_editor_property("HitSound").get_name(), "pitch", feedback.get_editor_property("HitSoundPitch"))

# --- 꾸민 게임 모드의 폰
for p in ("/Game/Showcase/Game/BP_FirstVRFactoryMode", "/Game/Showcase/Game/BP_FirstVRLabMode"):
    bp = load(p)
    cdo = unreal.get_default_object(bp_class(bp))
    cdo.modify()
    cdo.set_editor_property("default_pawn_class", bp_class(pawn))
    compile_save(bp)
    print(p.split("/")[-1], "pawn", unreal.get_default_object(bp_class(bp)).get_editor_property("default_pawn_class").get_name())
