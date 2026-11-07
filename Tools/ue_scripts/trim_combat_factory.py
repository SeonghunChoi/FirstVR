"""8-3 성능: L_Combat_Factory 가볍게 하기.

- 환경 복사본(L_Combat_Factory_Env)에서 전투 자리(A 마당 가운데)로부터 RADIUS m보다 먼 액터를 지운다.
  하늘선을 만드는 배경 카드(sm_en_Background_*)는 남긴다.
- 해(DirectionalLight) 동적 그림자 거리를 SHADOW_M m로, 캐스케이드를 2개로.
- 구름(VolumetricCloud)을 지운다(하늘 구체는 남김).
"""
import math
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

CENTER = (1250, -1650)
RADIUS = 40
SHADOW_M = 40
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

removed = 0
env_pkg = None
for a in EAS.get_all_level_actors():
    lvl = a.get_level().get_outer().get_name()
    if lvl != "L_Combat_Factory_Env":
        continue
    env_pkg = a.get_outermost()
    if a.get_actor_label().startswith("sm_en_Background"):
        continue
    o, e = a.get_actor_bounds(False)
    dx = max(abs(o.x - CENTER[0]) - e.x, 0); dy = max(abs(o.y - CENTER[1]) - e.y, 0)
    if math.hypot(dx, dy) / 100 > RADIUS:
        EAS.destroy_actor(a); removed += 1
print("removed far env actors", removed)

for a in EAS.get_all_level_actors():
    if a.get_level().get_outer().get_name() != "L_Combat_Factory":
        continue
    cn = a.get_class().get_name()
    if cn == "DirectionalLight":
        c = a.get_component_by_class(unreal.DirectionalLightComponent)
        c.set_editor_property("dynamic_shadow_distance_movable_light", SHADOW_M * 100)
        c.set_editor_property("dynamic_shadow_cascades", 2)
        print("sun shadow", c.get_editor_property("dynamic_shadow_distance_movable_light"), c.get_editor_property("dynamic_shadow_cascades"))
    elif cn == "VolumetricCloud":
        EAS.destroy_actor(a); print("removed VolumetricCloud")

LES.save_current_level()
if env_pkg:
    print("saved env", unreal.EditorLoadingAndSavingUtils.save_packages([env_pkg], False))
print("dirty", [p.get_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()])
