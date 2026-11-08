"""8-6b: 구간 표식 머티리얼 — 빛나는 언릿 하늘색(M_SectionMarker)을 만들어 BP_EncounterSection의 Marker에 넣는다.
기본 머티리얼(회색 체커)은 연구소 회색 바닥에서 거의 보이지 않았다.

    MSYS_NO_PATHCONV=1 python Tools/ue_remote.py Tools/ue_scripts/make_section_marker_material.py
"""
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

require_editing()

PATH = "/Game/Portfolio/Game/M_SectionMarker"
SECTION = "/Game/Portfolio/Game/BP_EncounterSection"
MEL = unreal.MaterialEditingLibrary

if EAL.does_asset_exist(PATH):
    mat = load(PATH)
else:
    folder, name = PATH.rsplit("/", 1)
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    color = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, 0)
    color.set_editor_property("constant", unreal.LinearColor(0.1, 1.0, 1.6, 1.0))   # 1보다 커서 블룸으로 빛나 보인다
    MEL.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.recompile_material(mat)
    EAL.save_loaded_asset(mat, only_if_is_dirty=False)
    print("created", PATH)

sec = load(SECTION)
sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
lib = unreal.SubobjectDataBlueprintFunctionLibrary
for h in sds.k2_gather_subobject_data_for_blueprint(sec):
    data = lib.get_data(h)
    if str(lib.get_variable_name(data)) == "Marker":
        m = lib.get_object_for_blueprint(data, sec)
        m.modify()
        m.set_editor_property("override_materials", [mat])
        m.set_editor_property("cast_shadow", False)
        print("marker material", m.get_editor_property("override_materials"))
compile_save(sec)
