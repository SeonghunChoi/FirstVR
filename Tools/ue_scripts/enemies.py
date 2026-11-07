"""실행 중인 월드의 적 위치와 속도."""
import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Character):
    if "Enemy" in a.get_class().get_name():
        l = a.get_actor_location(); v = a.get_velocity()
        print(a.get_name(), (round(l.x), round(l.y), round(l.z)), "speed", round(v.length()))
