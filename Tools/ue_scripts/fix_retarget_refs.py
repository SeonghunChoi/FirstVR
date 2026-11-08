"""8-5c 바로잡기: 리타깃한 블렌드 스페이스·몽타주가 옮기기 전 경로(/Game/X_UE4)를 가리키던 것을 Showcase/Enemy/Anims/로 다시 잇는다.

8-5a의 retarget_enemy_anims.py는 결과를 하나씩 옮기고 저장했는데, 블렌드 스페이스·몽타주를 먼저 저장한 뒤 시퀀스를 옮겨서
메모리에서만 참조가 고쳐지고 파일에는 옛 경로가 남았다(리다이렉터도 지웠음). 에디터를 다시 켜면 샘플·세그먼트가 비어 있었다.
원본(UE5) 에셋의 SampleData / SlotAnimTracks를 읽어 시퀀스 경로를 _UE4 사본으로 바꿔 넣고 저장한다.
"""
import re
import sys; sys.path.insert(0, r"D:\Projects\Unreal\FirstVR\Tools\ue_scripts")
from fv import *

OUT = "/Game/Showcase/Enemy/Anims"
PAIRS = [("/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run", f"{OUT}/BS_Idle_Walk_Run_UE4", "SampleData")]
PAIRS += [(f"/Game/Portfolio/Enemies/Attack/AM_Attack_0{i}", f"{OUT}/AM_Attack_0{i}_UE4", "SlotAnimTracks") for i in (1, 2, 3)]
REF = re.compile(r"'/Game/[^'.]+/([^'./]+)\.\1'")


def remap(m):
    name = m.group(1) + "_UE4"
    path = f"{OUT}/{name}"
    if not EAL.does_asset_exist(path):
        raise RuntimeError(f"missing {path}")
    return f"'{path}.{name}'"


for src, dst, prop in PAIRS:
    text = G.get_property_by_path(load(src), prop)
    new = REF.sub(remap, text)
    asset = load(dst)
    set_path(asset, prop, new)
    EAL.save_loaded_asset(asset, only_if_is_dirty=False)
    after = G.get_property_by_path(asset, prop)
    refs = re.findall(r"'(/Game/[^']+)'", after)
    print(dst.split("/")[-1], len(refs), "refs, all in Anims:", all(r.startswith(OUT) for r in refs))
