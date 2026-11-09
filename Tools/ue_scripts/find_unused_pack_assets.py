"""Fab 팩 폴더에서 우리 콘텐츠가 쓰지 않는 에셋을 찾는다(읽기만 함).

    python Tools/ue_remote.py Tools/ue_scripts/find_unused_pack_assets.py

뿌리 = 우리 콘텐츠(Portfolio, Showcase)와 git에 있는 템플릿 폴더, 그리고 프로젝트 설정이 가리키는 맵·게임 모드.
뿌리에서 패키지 의존성(하드 + 소프트)을 따라가 닿는 에셋은 '쓰임', 팩 폴더에서 닿지 않는 에셋은 '안 쓰임'.
결과는 폴더별 요약을 출력하고, 안 쓰는 패키지 목록을 Saved/UnusedPackAssets/<폴더>.txt에 쓴다.
"""
import os
import unreal

AR = unreal.AssetRegistryHelpers.get_asset_registry()
AR.search_all_assets(True)
CONTENT = r"D:\Projects\Unreal\FirstVR\Content"
OUT = r"D:\Projects\Unreal\FirstVR\Saved\UnusedPackAssets"

ROOT_FOLDERS = ["/Game/Portfolio", "/Game/Showcase", "/Game/Characters", "/Game/LevelPrototyping", "/Game/VRTemplate",
                "/Game/XRFramework", "/Game/XRMannequins", "/Game/VRSpectator", "/Game/Weapons"]
PACKS = ["AbandonedFactory", "FPS_Assault_Pack", "Nimikko_WesternTown", "Sci-Fi_Shots_Pack2_Game_Of_Weapons", "SciFiWorld",
         "SciFiSoldier02", "WastelandWarrior", "TF_Russian_Pistols", "Realistic_Starter_VFX_Pack_Vol2", "FreeWeaponSFX",
         "FreeTPPMeleeSFX", "GeneralWhooshSFX", "DamageVocalizations"]


def packages_under(path):
    return {str(a.package_name) for a in AR.get_assets_by_path(path, recursive=True)}


opts = unreal.AssetRegistryDependencyOptions(include_soft_package_references=True, include_hard_package_references=True,
                                             include_searchable_names=False, include_soft_management_references=False,
                                             include_hard_management_references=False)
roots = set()
for f in ROOT_FOLDERS:
    roots |= packages_under(f)

seen = set()
stack = list(roots)
while stack:
    p = stack.pop()
    if p in seen:
        continue
    seen.add(p)
    deps = AR.get_dependencies(p, opts) or []
    for d in deps:
        d = str(d)
        if d.startswith("/Game/") and d not in seen:
            stack.append(d)


def disk_size(pkg):
    rel = pkg[len("/Game/"):].replace("/", os.sep)
    for ext in (".uasset", ".umap"):
        f = os.path.join(CONTENT, rel + ext)
        if os.path.exists(f):
            s = os.path.getsize(f)
            u = os.path.join(CONTENT, rel + ".ubulk")
            return s + (os.path.getsize(u) if os.path.exists(u) else 0)
    return 0


os.makedirs(OUT, exist_ok=True)
print("roots", len(roots), "reachable", len(seen))
print("%-36s %6s %6s %9s %9s" % ("pack", "total", "unused", "used MB", "unused MB"))
for pack in PACKS:
    pk = packages_under("/Game/" + pack)
    unused = sorted(pk - seen)
    used = pk & seen
    us = sum(disk_size(p) for p in used) / 1e6
    un = sum(disk_size(p) for p in unused) / 1e6
    print("%-36s %6d %6d %9.1f %9.1f" % (pack, len(pk), len(unused), us, un))
    with open(os.path.join(OUT, pack + ".txt"), "w", encoding="utf-8") as fp:
        fp.write("\n".join(unused))
    with open(os.path.join(OUT, pack + ".used.txt"), "w", encoding="utf-8") as fp:
        fp.write("\n".join(sorted(used)))
