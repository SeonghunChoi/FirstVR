"""8-6e: 승리 화면에 '다음 레벨' 버튼 — WBP_VictoryScreen의 RestartButton 아래에 NextLevelButton(+ NextLevelText)을 더한다.

    python Tools/ue_remote.py Tools/ue_scripts/make_next_level_button.py

엔진 Python에는 위젯 트리를 고치는 API가 없지만, 트리 안 위젯은 경로(:WidgetTree.<이름>)로 찾을 수 있고
new_object로 위젯을 만들어 패널의 add_child(BlueprintCallable)로 붙일 수 있다. 모양은 RestartButton·ButtonText를 따른다.
보이기·글자·누름은 C++ 부모 UEncounterResultWidget이 맡는다(BindWidgetOptional, 게임 모드에 다음 레벨이 없으면 접힘).
이미 있으면 멈춘다. 실패 화면에는 넣지 않는다(실패하면 다시 시작만).
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

WBP = "/Game/Portfolio/UI/WBP_VictoryScreen"
w = load(WBP)
tree = unreal.find_object(None, w.get_path_name() + ":WidgetTree")
restart = unreal.find_object(None, w.get_path_name() + ":WidgetTree.RestartButton")
restart_text = unreal.find_object(None, w.get_path_name() + ":WidgetTree.ButtonText")
if unreal.find_object(None, w.get_path_name() + ":WidgetTree.NextLevelButton"):
    raise RuntimeError("NextLevelButton already exists")

w.modify()
tree.modify()
box = restart.get_parent()

button = unreal.new_object(unreal.Button, outer=tree, name="NextLevelButton")
button.set_editor_property("widget_style", restart.get_editor_property("widget_style"))
set_path(button, "bIsVariable", "true")
slot = box.add_child(button)
rs = restart.get_editor_property("slot")
slot.set_editor_property("horizontal_alignment", rs.get_editor_property("horizontal_alignment"))
slot.set_editor_property("vertical_alignment", rs.get_editor_property("vertical_alignment"))
slot.set_editor_property("padding", unreal.Margin(0, 16, 0, 0))

text = unreal.new_object(unreal.TextBlock, outer=tree, name="NextLevelText")
for k in ("font", "color_and_opacity", "justification"):
    text.set_editor_property(k, restart_text.get_editor_property(k))
text.set_editor_property("text", "Next Level")
set_path(text, "bIsVariable", "true")
bslot = button.add_child(text)
rts = restart_text.get_editor_property("slot")
for k in ("horizontal_alignment", "vertical_alignment", "padding"):
    bslot.set_editor_property(k, rts.get_editor_property(k))

for i in range(box.get_children_count()):
    print(i, box.get_child_at(i).get_name())
compile_save(w)
