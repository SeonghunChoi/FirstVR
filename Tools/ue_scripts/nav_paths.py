"""스폰 지점마다 플레이어 쪽(테이블 앞)까지 내비 경로를 찾는다."""
import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ps = [a for a in EAS.get_all_level_actors() if a.get_class().get_name() == "PlayerStart"][0].get_actor_location()
for a in EAS.get_all_level_actors():
    if a.get_class().get_name() == "EnemySpawnPoint":
        s = a.get_actor_location() + unreal.Vector(0, 0, 90)
        p = unreal.NavigationSystemV1.find_path_to_location_synchronously(w, s, ps + unreal.Vector(0, 0, 90))
        pts = p.get_editor_property("path_points") if p else []
        L = sum((pts[i + 1] - pts[i]).length() for i in range(len(pts) - 1)) if len(pts) > 1 else 0
        print(a.get_actor_label(), "valid", bool(p and p.is_valid()), "partial", bool(p and p.is_partial()),
              "len m", round(L / 100, 1), "end", (round(pts[-1].x), round(pts[-1].y)) if pts else None)
