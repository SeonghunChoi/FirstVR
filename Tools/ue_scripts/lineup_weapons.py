"""9-2b S5: 그레이박스 무기와 꾸민 자식 무기를 바닥에 나란히 눕혀 놓는다(사진용 임시 액터, 레벨은 저장하지 않는다).

    python Tools/ue_remote.py Tools/ue_scripts/lineup_weapons.py <x> <y> [roll]     # 줄 세우기
    python Tools/ue_remote.py Tools/ue_scripts/lineup_weapons.py clear              # 지우기

줄마다 같은 조합(권총·단발 소총·연발 소총), 왼쪽 열은 그레이박스, 오른쪽 열은 꾸민 자식 BP. 총구는 +X.
보이는 메시의 경계로 가운데를 맞추고 바닥(아래로 트레이스)에 내려놓는다.
"""
import sys
import unreal

TAG = "PortfolioLineup"
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()

ROWS = [
    ("/Game/Portfolio/Weapons/BP_PistolWeapon", "/Game/Showcase/Weapon/BP_PistolWeapon_PM"),
    ("/Game/Portfolio/Weapons/BP_RifleWeapon", "/Game/Showcase/Weapon/BP_RifleWeapon_AR"),
    ("/Game/Portfolio/Weapons/BP_AutoRifleWeapon", "/Game/Showcase/Weapon/BP_AutoRifleWeapon_RU74"),
]
COL_GAP = 120.0   # 열 사이(가운데 간격, cm)
ROW_GAP = 42.0    # 줄 사이


def clear():
    n = 0
    for a in unreal.GameplayStatics.get_all_actors_with_tag(world, TAG):
        a.destroy_actor(); n += 1
    print("cleared", n)


def visible_bounds(actor):
    """보이는 메시(스태틱·스켈레탈) 컴포넌트만 모은 경계. 잡기 충돌체·트리거는 뺀다."""
    lo = hi = None
    for c in actor.get_components_by_class(unreal.MeshComponent):
        if not c.is_visible() or c.get_owner() != actor:
            continue
        o, e, _ = unreal.SystemLibrary.get_component_bounds(c)
        a, b = o - e, o + e
        lo = a if lo is None else unreal.Vector(min(lo.x, a.x), min(lo.y, a.y), min(lo.z, a.z))
        hi = b if hi is None else unreal.Vector(max(hi.x, b.x), max(hi.y, b.y), max(hi.z, b.z))
    return lo, hi


def floor_z(x, y):
    r = unreal.SystemLibrary.line_trace_single(world, unreal.Vector(x, y, 300), unreal.Vector(x, y, -300),
                                               unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [],
                                               unreal.DrawDebugTrace.NONE, True)
    return r.to_tuple()[4].z if r and r.to_tuple()[0] else 0.0


if sys.argv[1] == "clear":
    clear()
else:
    clear()
    cx, cy = float(sys.argv[1]), float(sys.argv[2])
    roll = float(sys.argv[3]) if len(sys.argv) > 3 else -90.0   # -90: 위에서 보면 총의 위쪽이 화면 위(-Y)
    light_power = float(sys.argv[4]) if len(sys.argv) > 4 else 3000.0
    fz = floor_z(cx, cy)
    # 바탕판: 회색 그레이박스와 어두운 꾸민 무기가 모두 보이게 중간보다 어두운 무광 판
    plane = EAS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(cx, cy, fz + 1.0), unreal.Rotator(0, 0, 0), transient=True)
    plane.tags = [TAG]
    smc = plane.static_mesh_component
    smc.set_mobility(unreal.ComponentMobility.MOVABLE)
    smc.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Plane"))
    plane.set_actor_scale3d(unreal.Vector(2.8, 1.5, 1))
    mid_mat = smc.create_dynamic_material_instance(0, unreal.load_asset("/Engine/BasicShapes/BasicShapeMaterial"))
    mid_mat.set_vector_parameter_value("Color", unreal.LinearColor(0.045, 0.05, 0.058, 1))
    fz += 1.0
    # 사진 조명: 바탕판 위 넓은 사각 조명(그림자를 부드럽게)
    if light_power > 0:
        lamp = EAS.spawn_actor_from_class(unreal.RectLight, unreal.Vector(cx - 40, cy + 60, fz + 160), unreal.Rotator(0, -90, 0), transient=True)
        lamp.tags = [TAG]
        lc = lamp.get_component_by_class(unreal.RectLightComponent)
        lc.set_mobility(unreal.ComponentMobility.MOVABLE)
        lc.set_intensity(light_power)
        lc.set_editor_property("source_width", 150.0)
        lc.set_editor_property("source_height", 150.0)
        lc.set_editor_property("attenuation_radius", 600.0)
    for i, pair in enumerate(ROWS):
        for j, path in enumerate(pair):
            cls = unreal.EditorAssetLibrary.load_blueprint_class(path)
            a = EAS.spawn_actor_from_class(cls, unreal.Vector(0, 0, 500), unreal.Rotator(roll, 0, 0), transient=True)
            a.tags = [TAG]
            for p in a.get_components_by_class(unreal.PrimitiveComponent):
                p.set_simulate_physics(False)
            lo, hi = visible_bounds(a)
            mid = (lo + hi) * 0.5
            tx = cx + (j - 0.5) * COL_GAP
            ty = cy + (i - 1) * ROW_GAP
            a.set_actor_location(a.get_actor_location() + unreal.Vector(tx - mid.x, ty - mid.y, fz + 0.3 - lo.z), False, False)
            print(path.split("/")[-1], "size", round(hi.x - lo.x), round(hi.y - lo.y), round(hi.z - lo.z))
    print("floor", round(fz, 1))
