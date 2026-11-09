"""우리 콘텐츠의 의존성이 모두 디스크에 있는지 본다(팩에서 안 쓰는 에셋을 지운 뒤 확인용, 읽기만 함).

    python Tools/ue_remote.py Tools/ue_scripts/check_asset_refs.py

뿌리 폴더(우리 콘텐츠·템플릿) 패키지에서 의존성(하드 + 소프트)을 따라가며 /Game/ 의존성이 에셋 레지스트리에 있는지 확인한다.
없는 것은 '빠진 참조'로 누가 참조하는지와 함께 출력한다.
"""
import unreal

AR = unreal.AssetRegistryHelpers.get_asset_registry()
AR.search_all_assets(True)
ROOT_FOLDERS = ["/Game/Portfolio", "/Game/Showcase", "/Game/Characters", "/Game/LevelPrototyping", "/Game/VRTemplate",
                "/Game/XRFramework", "/Game/XRMannequins", "/Game/VRSpectator", "/Game/Weapons"]
opts = unreal.AssetRegistryDependencyOptions(include_soft_package_references=True, include_hard_package_references=True,
                                             include_searchable_names=False, include_soft_management_references=False,
                                             include_hard_management_references=False)
exists = {str(a.package_name) for a in AR.get_assets_by_path("/Game", recursive=True)}
stack = [p for f in ROOT_FOLDERS for p in {str(a.package_name) for a in AR.get_assets_by_path(f, recursive=True)}]
seen, missing = set(), {}
while stack:
    p = stack.pop()
    if p in seen:
        continue
    seen.add(p)
    for d in AR.get_dependencies(p, opts) or []:
        d = str(d)
        if not d.startswith("/Game/"):
            continue
        if d not in exists:
            missing.setdefault(d, set()).add(p)
        elif d not in seen:
            stack.append(d)
print("reachable", len(seen), "missing", len(missing))
for d, refs in sorted(missing.items()):
    print("MISSING", d, "<-", ", ".join(sorted(refs)[:3]))
