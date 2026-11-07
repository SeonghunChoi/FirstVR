"""에디터 안에서 실행되는 Python 스크립트들이 함께 쓰는 도우미.

스크립트 맨 앞에서:
    import sys; sys.path.insert(0, r"D:\\Projects\\Unreal\\FirstVR\\Tools\\ue_scripts")
    from fv import *
"""

import unreal

G = unreal.GraphAuthoringLibrary
BEL = unreal.BlueprintEditorLibrary
EAL = unreal.EditorAssetLibrary


def load(path):
    asset = unreal.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Asset not found: {path}")
    return asset


def load_cls(path):
    """'/Script/Engine.KismetMathLibrary' 같은 C++ 클래스나 '/Game/...BP_X.BP_X_C' 같은 BP 클래스."""
    c = unreal.load_class(None, path)
    if c is None:
        raise RuntimeError(f"Class not found: {path}")
    return c


def bp_class(bp):
    return BEL.generated_class(bp)


def graph(bp, name="EventGraph"):
    g = BEL.find_graph(bp, name)
    if g is None:
        raise RuntimeError(f"Graph '{name}' not found in {bp.get_name()}")
    return g


def dump_graph(g, pins=True):
    for n in G.get_nodes(g):
        print(G.describe_node(n))
        if pins:
            for p in G.describe_pins(n):
                print("    " + p)


def find_node(g, title_part):
    for n in G.get_nodes(g):
        if title_part.lower() in G.describe_node(n).lower():
            return n
    return None


def link(a, a_pin, b, b_pin):
    if not G.link_pins(a, a_pin, b, b_pin):
        raise RuntimeError(f"Link failed: {G.describe_node(a)}.{a_pin} -> {G.describe_node(b)}.{b_pin}")


def set_default(node, pin, value):
    if not G.set_pin_default(node, pin, str(value)):
        raise RuntimeError(f"Set default failed: {G.describe_node(node)}.{pin}")


def set_path(obj, path, value):
    if not G.set_property_by_path(obj, path, str(value)):
        raise RuntimeError(f"Set property failed: {obj.get_name()}.{path} = {value}")


def compile_save(bp):
    BEL.compile_blueprint(bp)
    EAL.save_loaded_asset(bp, only_if_is_dirty=False)
    print(f"Compiled and saved {bp.get_path_name()}")
