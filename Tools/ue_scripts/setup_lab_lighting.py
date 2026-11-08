"""8-6a: L_Lab 조명 — 팩 방의 Static 조명을 라이트맵으로 굽고 이 레벨만 Lumen을 끈다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/setup_lab_lighting.py setup
    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/setup_lab_lighting.py build [preview|medium|high|production]

setup: 팩의 포스트 프로세스 볼륨(첫 방만 덮음)을 무한 범위(unbound)로 바꾸고 GI 방법을 None(라이트맵 사용),
       반사 방법을 Screen Space(반사 캡처)로 덮어쓴다. 프로젝트 기본은 Lumen이라 다른 레벨은 그대로다.
       Lightmass Importance Volume을 두 방과 탈출 복도에 맞춰 놓는다(라이트맵 계산을 이 안에 집중).
build: 라이팅 빌드(Lightmass)와 반사 캡처 빌드. 오래 걸리면 에디터가 그동안 멈춘다. 끝나면 레벨을 저장한다.
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
MAP = "/Game/Showcase/Map/L_Lab"
# 두 방 + 게이트 + 탈출 복도: x -1500 ~ 4850, y -1400 ~ 1550, z -100 ~ 700
LMIV_MIN, LMIV_MAX = unreal.Vector(-1550, -1450, -100), unreal.Vector(4900, 1600, 700)

world = UES.get_editor_world()
if world.get_path_name().split(".")[0] != MAP:
    raise RuntimeError(f"open {MAP} first")
cmd = sys.argv[1] if len(sys.argv) > 1 else "setup"

if cmd == "setup":
    pp = [a for a in EAS.get_all_level_actors() if a.get_class().get_name() == "PostProcessVolume"][0]
    pp.set_editor_property("unbound", True)
    s = pp.get_editor_property("settings")
    s.set_editor_property("override_dynamic_global_illumination_method", True)
    s.set_editor_property("dynamic_global_illumination_method", unreal.DynamicGlobalIlluminationMethod.NONE)
    s.set_editor_property("override_reflection_method", True)
    s.set_editor_property("reflection_method", unreal.ReflectionMethod.SCREEN_SPACE)
    pp.set_editor_property("settings", s)
    print("postprocess unbound, GI", s.get_editor_property("dynamic_global_illumination_method"), "reflection", s.get_editor_property("reflection_method"))

    for a in EAS.get_all_level_actors():
        if a.get_class().get_name() == "LightmassImportanceVolume":
            EAS.destroy_actor(a)
    center = (LMIV_MIN + LMIV_MAX) * 0.5
    size = LMIV_MAX - LMIV_MIN
    v = EAS.spawn_actor_from_class(unreal.LightmassImportanceVolume, center)
    o, e = v.get_actor_bounds(False)   # 기본 브러시 크기(보통 200cm 정육면체)에 맞춰 배율
    v.set_actor_scale3d(unreal.Vector(size.x / (2 * e.x), size.y / (2 * e.y), size.z / (2 * e.z)))
    v.set_actor_label("LightmassImportanceVolume_Lab")
    v.set_folder_path("Lighting")
    o, e = v.get_actor_bounds(False)
    print("lightmass importance volume", o - e, o + e)
    LES.save_current_level()

elif cmd == "build":
    quality = {"preview": unreal.LightingBuildQuality.QUALITY_PREVIEW, "medium": unreal.LightingBuildQuality.QUALITY_MEDIUM,
               "high": unreal.LightingBuildQuality.QUALITY_HIGH, "production": unreal.LightingBuildQuality.QUALITY_PRODUCTION}[
        sys.argv[2] if len(sys.argv) > 2 else "preview"]
    ok = LES.build_light_maps(quality, True)
    print("build_light_maps", quality, ok)
    LES.save_current_level()
    print("saved")
