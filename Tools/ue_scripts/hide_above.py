"""9-2b: 위에서 찍을 때 천장을 잠깐 숨긴다(에디터 화면에서만 숨김, 레벨은 바뀌지 않음, 저장 안 함).

    python Tools/ue_remote.py Tools/ue_scripts/hide_above.py <높이cm>     # 그 높이보다 위에 놓인 스태틱 메시 액터를 숨김
    python Tools/ue_remote.py Tools/ue_scripts/hide_above.py show         # 숨긴 것을 모두 다시 보임
"""
import sys
import unreal

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
arg = sys.argv[1]
n = 0
for a in EAS.get_all_level_actors():
    if not isinstance(a, unreal.StaticMeshActor):
        continue
    if arg == "show":
        if a.is_temporarily_hidden_in_editor():
            a.set_is_temporarily_hidden_in_editor(False); n += 1
    elif a.get_actor_location().z > float(arg):
        a.set_is_temporarily_hidden_in_editor(True); n += 1
print(("shown" if arg == "show" else "hidden"), n)
