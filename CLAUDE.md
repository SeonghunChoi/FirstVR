# CLAUDE.md — FirstVR (젠틀몬스터 VR 포트폴리오)

이 파일은 Claude Code가 이 프로젝트에서 작업할 때 따르는 안내서다.
프로젝트 전체 계획과 설계 결정의 근거는 `Docs/PLAN.md`(기존 계획 문서)를 참고한다.

---

## 1. 사용자와 프로젝트

- 사용자는 **유니티 경력 약 19년**의 클라이언트 개발자다. C#, 컴포넌트 설계, DOTS/ECS, 전투 시스템에 익숙하다.
- **언리얼은 익숙하지 않다.** 파일 추가, 빌드, 에디터 조작, 리플렉션 매크로 등은 처음부터 설명한다.
- 목표: UE5 VR 슈팅 포트폴리오. "언리얼을 언리얼답게 쓴다"는 것을 증명하는 **컴포넌트 조합형 무기 시스템**이 핵심이다.
- 프로젝트: VR 템플릿 기반 `FirstVR` (현재 블루프린트 전용 → 첫 C++ 클래스 추가 시 C++ 프로젝트로 전환됨)
- 헤드셋 없이 개발: Meta XR Simulator를 단독 OpenXR 런타임으로 사용 중 (에디터 VR Preview).

---

## 2. 진행 방식: 튜토리얼 모드

### 2.1 기본 규칙

- **한 번에 한 스텝만** 진행한다. 스텝이 끝나면 사용자의 확인("됐어", "다음")을 기다린다.
- 각 스텝은 아래 순서로 안내한다.
  1. **목표**: 이번 스텝에서 만드는 것 (한두 줄)
  2. **왜**: 설계 결정과의 연결 (`Docs/PLAN.md`의 해당 절)
  3. **에디터에서 할 일**: 메뉴 경로를 정확히 (`Tools > New C++ Class...`처럼)
  4. **코드**: 파일 경로와 전체 내용. 핵심 줄은 주석 대신 설명으로 짚는다.
  5. **빌드**: 이번 변경에 맞는 빌드 방법 (3.4 참고)
  6. **확인 방법**: 무엇이 보이면 성공인지 (로그 문구, 화면 상태)
- 언리얼 개념은 **유니티 대응 개념과 비교**해서 설명한다. (예: `UActorComponent` ≈ `MonoBehaviour`, `UDataAsset` ≈ `ScriptableObject`, `BeginPlay` ≈ `Start`)
- 코드는 Claude가 작성하고, 에디터 조작(데이터 에셋 생성, 블루프린트 설정, 레벨 배치)은 사용자가 직접 한다.
- 사용자가 "직접 해볼게"라고 하면 코드를 주지 말고 **목표, 필요한 API 이름, 힌트**만 준다.

### 2.2 설계 판단이 필요할 때

- 구현 중 설계 결정이 필요한 지점(예: VR 템플릿의 잡기 시스템을 재사용할지)에서는 **바로 답을 주지 말고 핵심을 짚는 질문을 하나씩** 던져 사용자가 결정하게 한다.
- 사용자가 "정리해줘" 또는 "결론 내줘"라고 하면 질문을 멈추고 정리한다.
- 결정된 내용은 `Docs/PLAN.md`의 해당 절과 이 파일의 **8. 진행 상황**에 반영한다.

### 2.3 막혔을 때

- 빌드 에러는 에러 메시지 전체를 요청하고, 원인을 언리얼 빌드 구조(UHT, 모듈, 링크) 관점에서 설명한 뒤 수정한다.
- 에디터 화면 관련 문제는 스크린샷이나 메뉴 상태를 물어본다. 추측으로 메뉴 경로를 지어내지 않는다.

---

## 3. 언리얼 기초 안내 (튜토리얼 중 필요할 때 참조)

### 3.1 개발 환경 준비 (최초 1회)

- **Visual Studio 2022** 설치 시 워크로드 **"C++를 사용한 게임 개발"**을 선택하고, 오른쪽 세부 항목에서 Unreal Engine 관련 구성 요소(Unreal Engine 설치 관리자, IDE 지원 등)를 체크한다. MSVC 빌드 도구와 Windows SDK가 함께 설치되어야 한다.
- JetBrains Rider를 써도 된다. 이 경우 3.2의 `.sln` 대신 `.uproject`를 바로 열 수 있다.
- 사용하는 UE 버전과 설치 경로를 **8. 진행 상황**의 환경 정보에 기록해 둔다.

### 3.2 첫 C++ 클래스 추가 (블루프린트 프로젝트 → C++ 프로젝트 전환)

1. 에디터에서 `Tools > New C++ Class...`
2. 부모 클래스 선택 (예: `Actor Component`). 목록에 없으면 상단 `All Classes` 탭에서 검색.
3. 이름과 경로 입력. **Public/Private 분리**를 선택하고 폴더를 지정한다. (예: `Weapon/`)
4. `Create Class`를 누르면 언리얼이 다음을 생성한다.
   - `Source/FirstVR/` 모듈 폴더, `FirstVR.Build.cs`, `FirstVR.Target.cs`, `FirstVREditor.Target.cs`
   - 프로젝트 루트에 `FirstVR.sln`
5. 첫 전환 후에는 **에디터를 닫고** IDE에서 빌드한 뒤 `.uproject`로 다시 여는 것이 안전하다.

### 3.3 에디터 밖에서 파일을 추가하는 방법

Claude Code가 파일을 직접 만들 때는 이 방식을 쓴다.

1. `Source/FirstVR/Public/<폴더>/XXX.h`, `Source/FirstVR/Private/<폴더>/XXX.cpp` 생성
2. **프로젝트 파일 재생성**: 탐색기에서 `FirstVR.uproject` 우클릭 → `Generate Visual Studio project files` (Windows 11은 `추가 옵션 표시` 안에 있다)
3. 에디터를 닫은 상태에서 빌드 (3.4)

### 3.4 빌드 방법 선택

| 변경 내용 | 빌드 방법 |
|---|---|
| 새 파일/새 클래스 추가, `UCLASS`/`USTRUCT`/`UENUM` 추가, `UPROPERTY`/`UFUNCTION` 추가·변경, `Build.cs` 수정 | **에디터를 닫고** IDE 또는 커맨드라인으로 전체 빌드 |
| `.cpp` 함수 본문만 수정 | 에디터 실행 중 **Live Coding** (`Ctrl+Alt+F11` 또는 하단 툴바의 컴파일 버튼) |

커맨드라인 빌드 (Claude Code가 실행할 때, 에디터는 닫혀 있어야 한다):

```bat
"<UE설치경로>\Engine\Build\BatchFiles\Build.bat" FirstVREditor Win64 Development -Project="<프로젝트경로>\FirstVR.uproject" -WaitMutex
```

- 에디터가 열린 채 Live Coding이 켜져 있으면 외부 빌드가 거부된다. 이때는 에디터를 닫으라고 안내한다.
- `<UE설치경로>`, `<프로젝트경로>`는 **8. 진행 상황**의 환경 정보를 사용한다.

### 3.5 언리얼 C++ 규칙 요약

- **이름 접두사**: `A`(Actor), `U`(UObject/컴포넌트/데이터 에셋), `F`(구조체), `E`(열거형), `I`(인터페이스)
- 헤더 마지막 include는 반드시 `#include "XXX.generated.h"`
- 다른 모듈에 노출할 클래스는 `FIRSTVR_API` 매크로를 붙인다.
- 리플렉션 매크로: `UCLASS()`, `USTRUCT()`, `UENUM()`, 클래스 본문 첫 줄 `GENERATED_BODY()`
- 로그: 전용 카테고리 `LogWeapon`을 만들어 `UE_LOG(LogWeapon, Log, TEXT("..."))`로 출력. 에디터 `Window > Output Log`에서 확인.
- 모듈 의존성이 필요하면 `FirstVR.Build.cs`의 `PublicDependencyModuleNames`에 추가하고 전체 빌드한다.

### 3.6 에디터 작업 자주 쓰는 경로

- **데이터 에셋 만들기**: 콘텐츠 브라우저 우클릭 → `Miscellaneous > Data Asset` → 클래스 선택
- **C++ 클래스를 부모로 블루프린트 만들기**: 콘텐츠 브라우저의 `C++ Classes` 폴더에서 클래스 우클릭 → `Create Blueprint class based on ...`
- **블루프린트에 컴포넌트 추가**: 블루프린트 에디터 `Components` 패널 → `+ Add` → 클래스 검색
- **VR 테스트**: 플레이 버튼 옆 `⋮` → `VR Preview` (시뮬레이터 런타임이 켜져 있어야 활성화)
- **일반 테스트**: 플레이 버튼 (VR 없이 로직 검증용)

---

## 4. 설계 규칙 (반드시 지킬 것)

`Docs/PLAN.md` 4~5장의 결정 사항이다. 코드를 작성할 때 이 규칙을 어기면 사용자에게 먼저 알린다.

1. **코어(`AWeaponBase`)는 구체 단계 클래스를 참조하지 않는다.** `UWeaponStepComponent`(추상)만 안다. `#include "MagazineStep.h"` 같은 줄이 코어에 생기면 설계 위반이다.
2. **단계 컴포넌트는 "다음 단계"를 모른다.** 순서는 `UWeaponDataAsset::FireSequence`에만 있다.
3. 단계는 실행 결과로 **`Success` / `Failed` / `Pending`** 중 하나를 델리게이트로 발행한다. 게임 규칙상의 거절은 Error가 아니라 Failed다.
4. **구체적인 피드백(빈 격발음, 잠김 표시)은 해당 단계 컴포넌트가 책임진다.** 코어는 처리하지 않는다.
5. **`FFireContext`에는 코어와 공유해야 하는 정보만** 넣고, 칸 이름은 코어가 쓰는 의미로 짓는다. (`DrawRatio` ✗ → `DamageScale` ✓)
6. 데이터 위치: 무기 공통값은 무기 데이터 에셋, 탄을 따라가는 값은 탄 데이터 에셋, 맞은 부위를 따라가는 값은 적 데이터.
7. 노출 범위:
   - 무기 공통 설정값: `EditDefaultsOnly`, `BlueprintReadOnly`
   - 배치별 초기값: `EditInstanceOnly` (스폰용은 `ExposeOnSpawn`)
   - 실행 중 상태값: `VisibleInstanceOnly`, `BlueprintReadOnly`, 변경은 `BlueprintCallable` 함수로만
8. **메시 의존 금지**: 발사 원점, 파지 위치, 탄창 결합부는 이름이 정해진 소켓 또는 Scene Component로 찾는다. 지금은 기본 도형/템플릿 메시로 진행하고 나중에 Fab 에셋으로 교체한다.
9. 무기 시스템과 적 시스템은 `AProjectile`이 탄 데이터와 `DamageScale`을 전달하는 지점에서만 만난다. 피해 계산은 `UDamageReceiver`에서 한다.

---

## 5. 폴더 구조 (목표)

```
Source/FirstVR/
├── Public/
│   ├── Weapon/
│   │   ├── WeaponTypes.h          // EStepResult, FFireContext, LogWeapon
│   │   ├── WeaponBase.h           // AWeaponBase
│   │   ├── WeaponDataAsset.h      // UWeaponDataAsset
│   │   ├── AmmoDataAsset.h        // UAmmoDataAsset
│   │   ├── WeaponProjectile.h     // AProjectile 역할 (이름 충돌 방지용)
│   │   └── Steps/
│   │       ├── WeaponStepComponent.h
│   │       ├── MagazineStep.h
│   │       ├── GripStep.h
│   │       └── ...
│   └── Enemy/
│       ├── DamageReceiver.h
│       ├── EnemyDataAsset.h
│       └── HitReaction.h
└── Private/   (Public과 같은 구조로 .cpp)

Content/Portfolio/
├── Weapons/     // 무기 블루프린트
├── Data/        // 무기·탄·적 데이터 에셋
├── Enemies/
└── Maps/        // 테스트 레벨
```

- 직접 만든 콘텐츠는 `Content/Portfolio/` 아래에만 둔다. Fab 에셋과 섞지 않기 위해서다.

---

## 6. 튜토리얼 커리큘럼

각 스텝의 세부 내용은 진행 시점에 위 2.1 형식으로 풀어서 안내한다.

### Step 0. 저장소 준비
- 0-1. 프로젝트 루트에서 `git init`, 언리얼용 `.gitignore` 작성 (`Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/`, `*.sln` 등)
- 0-2. Fab 에셋 폴더를 `.gitignore`에 추가 (라이선스상 원본 재배포 제한). 실제 폴더 이름은 사용자에게 확인.
- 0-3. `Docs/PLAN.md` 배치, 첫 커밋

### Step 1. 흐름 뼈대 (VR 불필요)
- 1-1. 첫 C++ 클래스 추가로 C++ 프로젝트 전환 (3.2). `WeaponTypes.h`에 `LogWeapon`, `EStepResult`, `FFireContext`
- 1-2. `UWeaponStepComponent` 추상 클래스: `Execute(FFireContext&)`, `Report(EStepResult)`, `OnStepResult` 델리게이트, `OnFailedFeedback` (`BlueprintImplementableEvent`)
- 1-3. `UWeaponDataAsset`: `FireSequence`(`TArray<TSubclassOf<UWeaponStepComponent>>`), `Recoil`, `FireRate`
- 1-4. `AWeaponBase`: `BeginPlay`에서 `FireSequence` 순서대로 소유 컴포넌트를 찾아 `Steps` 구성, `RequestFire()`(`BlueprintCallable`), 결과 구독과 진행/중단/대기 처리
- 1-5. 검증용 더미 단계: 항상 Success / 항상 Failed / 1초 후 Success(Pending)
- 1-6. 에디터: 데이터 에셋 생성, 무기 블루프린트에 더미 단계 부착, 레벨 블루프린트에서 키 입력으로 `RequestFire` 호출
- ✅ 마일스톤: 출력 로그에서 순서대로 실행, Failed에서 중단, Pending 후 재개가 확인된다.

### Step 2. 탄창과 탄
- 2-1. `UAmmoDataAsset`: `Speed`, `Penetration`, `BaseDamage`
- 2-2. `UMagazineStep`: `Capacity`, `AllowedAmmo`, `InitialAmmo` → `BeginPlay`에서 `CurrentAmmo`로, `AddAmmo()` 규칙 검사, 탄 소모와 `Context.Ammo` 기록, 탄 없으면 Failed + 빈 격발 피드백
- 2-3. 투사체: `UProjectileMovementComponent`로 탄속 연결, 코어가 발사 원점에서 스폰
- ✅ 마일스톤: 키 입력 → 투사체 발사 → 탄 소진 시 Failed로 멈춤

### Step 3. VR 파지 (시뮬레이터 필요)
- 3-1. VR 템플릿의 잡기 구조 분석 (블루프린트 기반 `GrabComponent` 계열)
- 3-2. **설계 판단**: 템플릿 잡기를 재사용할지, C++로 새로 만들지 → 2.2 방식으로 질문
- 3-3. `UGripStep`: 파지 상태 갱신, 손 없으면 Failed
- 3-4. 트리거 입력을 `RequestFire`에 연결 (Enhanced Input)
- 3-5. 그레이박스 권총 완성: 발사 원점 / 파지 / 탄창 결합부를 소켓 또는 Scene Component로
- ✅ 마일스톤: VR Preview에서 권총을 집고 트리거로 발사

### Step 4. 적 반응
- 4-1. `UEnemyDataAsset`: 본 이름 기반 부위 배율, 방어력, 절뚝임 기준
- 4-2. `UDamageReceiver`: `ReceiveHit(Hit, Ammo, DamageScale)`에서 피해 계산
- 4-3. `UHitReaction`: 맞은 본 아래만 물리 시뮬레이션 후 블렌딩 (`PhysicalAnimationComponent` 또는 `SetAllBodiesBelowSimulatePhysics`)
- 4-4. 상태 변화: 다리 피해 누적 시 애니메이션 블루프린트 상태 전환
- 4-5. 사망 처리
- ✅ 마일스톤: 마네킹의 부위별 피해와 물리 피격 반응 확인

### Step 5 이후
- `Docs/PLAN.md` Phase 3(확장성 증명), Phase 4(전투 구간)를 같은 방식으로 스텝화한다.

---

## 7. 커밋 규칙

- 스텝(또는 하위 스텝)이 끝나 사용자가 확인하면 커밋을 제안한다. 커밋은 사용자 확인 후 실행한다.
- 메시지 형식: `[Step 1-2] UWeaponStepComponent 추상 클래스 추가`

---

## 8. 진행 상황

Claude Code는 스텝이 끝날 때마다 이 절을 갱신한다.

### 환경 정보
- UE 버전: 5.7
- UE 설치 경로: `C:\Program Files\Epic Games\UE_5.7`
- 프로젝트 경로: `D:\Projects\Unreal\FirstVR`
- IDE: JetBrains Rider (`.uproject`를 바로 연다)
- git-lfs 3.5.1 (저장소에 `git lfs install`로 훅 설치됨)

### 완료
- [x] VR 템플릿으로 `FirstVR` 프로젝트 생성
- [x] Meta XR Simulator 단독 런타임 방식 동작 확인
- [x] Phase 1 설계 완료 (`Docs/PLAN.md` 4~5장)
- [x] Step 0-1: `.gitignore` 보강(IDE/OS 항목), `.gitattributes`에 LFS 설정 (커밋 전)
- [x] Step 0-2: Fab 폴더(`FPS_Assault_Pack`, `Nimikko_WesternTown`) `.gitignore` 추가, `FPS_Assault_Pack` 추적 해제
- [x] Step 0-3: 초기 커밋을 LFS 적용·Fab 제외 상태로 다시 만듦 (`[Step 0] 저장소 초기화`, 에셋 186개 LFS)

### 현재 위치
- **Step 1-1** 시작 전
- 원격 저장소 없음. 옛 초기 커밋 데이터가 `.git`에 남아 있음 (정리는 사용자 확인 후 `git reflog expire` + `git gc`)

### 결정 기록
- 2026-10-29: 바이너리 에셋(`.uasset`, `.umap`, 원본 아트/미디어)은 Git LFS로 관리한다.
- 2026-10-29: Fab 에셋 폴더는 `Content/FPS_Assault_Pack/`, `Content/Nimikko_WesternTown/`. 저장소에서 제외하고, 새 Fab 에셋을 받으면 `.gitignore`에 폴더를 추가한다.
