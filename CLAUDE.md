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
  - 조작은 **시뮬레이터 창에 포커스**를 두고 한다. 언리얼 프리뷰 창에 포커스가 있으면 키가 먹지 않는다.
  - 시뮬레이터에 "No frames received"가 뜨면 `편집 > 에디터 개인설정 > 일반 > 퍼포먼스`에서 `백그라운드에서 CPU 덜 사용`을 끈다.
  - 시뮬레이터 키 (`%APPDATA%\MetaXR\MetaXrSimulator\keybindings.json`): `U` Grip(잡기), `T` 또는 마우스 왼쪽(Point & click) 트리거, WASD·`R`/`F` 이동, 방향키 회전, `Q`/`E` 기울기, 우클릭 드래그 시점, `]`/`[` 조작 대상 전환, `Space` 손 위치 초기화
  - 시뮬레이터 기본 손 자세는 오른손 손가락이 왼쪽을 향한다. 잡은 무기의 방향은 손 기준으로 판단한다.
  - `MotionVectorIndex == MotionVectorDepthIndex` 어서트 크래시: 시뮬레이터의 `XR_FB_space_warp` 때문. `DefaultEngine.ini`의 `[SystemSettings] xr.OpenXRAcquireMode=1`로 우회한다 (에디터 재시작 필요).
  - 시뮬레이터 `Viewport`(눈 선택)와 언리얼 프리뷰 창의 관전자 화면(왼쪽 눈)은 별개다.
  - 종료 시 `xrDestroySwapchain ... XR_ERROR_HANDLE_INVALID` ensure 로그는 무해하다.
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
- [x] Step 1-1: C++ 프로젝트 전환, `WeaponTypes.h`(`LogWeapon`, `EStepResult`, `FFireContext`) — Rider 빌드, 에디터에서 타입 노출 확인. `FFireContext::Ammo`는 Step 2-1에서 추가

- [x] Step 1-2: `UWeaponStepComponent` 추상 클래스 (`Execute`, 비가상 `Report`, `OnStepResult`, `OnFailedFeedback`) — 사용자 직접 구현(샘플 참고), Rider 빌드, `+ Add`에 노출되지 않음 확인

- [x] Step 1-3: `UWeaponDataAsset`(`UPrimaryDataAsset` 상속, `FireSequence`, `Recoil`, `FireRate`=초당 발사 수) — Rider 빌드, 에디터에서 데이터 에셋 생성·칸 노출 확인

- [x] Step 1-4: `AWeaponBase`(추상, `BuildSteps`, `RequestFire`, 현재 단계만 구독하며 Success 진행/Failed 중단/Pending 대기, `Fire()`는 로그만) — Rider 빌드 성공. `FireRate`·`Recoil` 적용은 Step 2-3

- [x] Step 1-5: 더미 단계 `UDebugSuccessStep`/`UDebugFailStep`/`UDebugPendingStep`(타이머, `Delay` 최소 0.1) — `Steps/DebugSteps.h` 한 파일, 커맨드라인 빌드 성공, `+ Add` 노출 확인

- [x] Step 1-6: 테스트 맵 `Portfolio/Maps/L_WeaponTest`, `Portfolio/Weapons/BP_TestWeapon`(더미 단계 3개, `WeaponData`=`DA_TestWeapon`), 레벨 블루프린트 `F` → `RequestFire`
- [x] ✅ **Step 1 마일스톤**: 순서 실행, Pending 후 재개, Failed 중단을 출력 로그로 확인 (테스트 A·B)

- [x] Step 2-1: `UAmmoDataAsset`(`Speed` cm/s, `Penetration`, `BaseDamage`), `FFireContext::Ammo`(전방 선언), `FIRE` 로그에 탄 이름 — Rider 빌드, `DA_Ammo_9mm` 생성 확인

- [x] Step 2-2: `UMagazineStep`(한 종류 탄, `AllowedAmmo[0]`로 초기 장전, `AddAmmo` 규칙 검사·넣은 수 반환, 탄 없으면 Failed), `BP_MagazineStep`(`OnFailedFeedback` → Print String), 부모에 `Blueprintable` 추가, 레벨 블루프린트 `R` → `AddAmmo` — 로그로 소모·Empty·장전 제한 확인
  - 피드백 로그 확인은 출력 로그 `Filters > Categories`에서 `LogWeapon` + `LogBlueprintUserMessages`

- [x] Step 2-3: `AWeaponProjectile`(추상, Sphere 충돌 + Mesh + `ProjectileMovement`, `Launch`로 탄속, 멈추면 Hit 로그 후 파괴, 수명 3초), `UAmmoDataAsset::ProjectileClass`, 코어가 `Root`/`Muzzle` 생성, `FireRate` 쿨다운(실제 발사 시각 기준), `Muzzle`에서 스폰 — 커맨드라인 빌드, `BP_Projectile_9mm` 발사·Hit·쿨다운 확인
  - `Recoil`은 `FIRE` 로그에 `Recoil × RecoilScale`로만 출력. 실제 적용은 Step 3(파지)
- [x] ✅ **Step 2 마일스톤**: 키 입력 → 투사체 발사 → 탄 소진 시 Failed로 멈춤

- [x] Step 3-1: 템플릿 잡기 구조 분석 — `BP_XRPawn`(손 근처 `BP_GrabComponent` 탐색 → `TryGrab`/`TryRelease`), `BP_GrabComponent`(SceneComponent, `bIsHeld`, `OnGrabbed`/`OnDropped`, `GetHeldByHand`, `E_GrabType`), `BP_Pistol`(잡으면 `IMC_Weapon_*` 추가, `IA_Shoot_*` → `MuzzleLocation`에서 스폰)
- [x] Step 3-2: 잡기 설계 결정 (아래 결정 기록, `Docs/PLAN.md` 4.7)
- [x] Step 3-3: `UGripStep`(추상, `bHeld`, `SetHeld`, `FindGripComponent`(const라 Pure 노드), `GripComponentName`="Grip", 손 없으면 Failed), `BP_GripStep`(BeginPlay에서 `BP_GrabComponent`의 `OnGrabbed`/`OnDropped` 구독), `BP_TestWeapon`에 `Grip`(`BP_GrabComponent`) 추가, `FireSequence`=[`BP_GripStep`, `MagazineStep`] — 일반 Play에서 Not held로 중단, VR Preview에서 Held/Released 확인
  - 추상 클래스는 `TSubclassOf` 선택 목록에 안 보이므로 데이터 에셋에는 `BP_GripStep`을 넣는다
  - VR Preview에서는 `F` 키가 다른 기능에 배정되어 있어 잡은 상태의 발사는 3-4(트리거)에서 확인
  - 3-3 이후 수정: 템플릿 잡기가 `Grip`의 부모만 손에 붙여 무기 일부만 따라오는 문제 → 코어 루트를 `Root`(Scene)에서 박스 충돌체 `Body`(`PhysicsActor`)로 교체, `BP_TestWeapon`은 `Body` 아래에 `Grip`·`Cube`(→`Handle`)·`Muzzle`, 남은 옛 `Root` 삭제 — Rider 빌드, VR Preview에서 무기 전체가 손을 따라옴 확인 (커밋 전)
  - VR Preview 중 키보드 입력은 레벨 블루프린트에 들어오지 않는다 (`C` 키도 무반응)
- [x] Step 3-4: `UTriggerInputComponent`(추상, 발사 단계 아님, `EnableForHand`/`DisableTrigger`, 잡은 손의 `IMC_Weapon_*` 추가 + `IA_Shoot_*` Started → `RequestFire`, `EndPlay`에서 해제), `Build.cs`에 `EnhancedInput`, 이름 찾기를 `FindSceneComponentByName`(`WeaponTypes`)으로 공용화, `BP_TriggerInput`(캐스트 결과를 `GrabComponent` 변수로 저장, `GetHeldByHand == Right`), `BP_TestWeapon`에 부착 — Rider 빌드, VR Preview에서 잡기 → 트리거 발사 → 탄 소진 Failed → 놓으면 입력 해제 확인
  - 블루프린트 노드 검색은 상황 맞춤이라, 다른 컴포넌트의 함수는 그 타입의 핀에서 끌어야 보인다
  - `DA_Ammo_9mm`의 `Speed`를 C++ 기본값 35000으로 되돌림 (테스트 중 100·1000으로 낮췄었음)
- [x] Step 3-5: 코어 `OnFired(Recoil)` 델리게이트(투사체 스폰 성공 시 발행), `UTriggerInputComponent`가 구독해 잡은 손에 `FireHaptic`(세기 0~1 제한) 재생, `BP_PistolWeapon`(`Body` 아래 `Grip`(Snap)·`Cube`(→`Grab`)·`MagazineWell`(자리만)·`Muzzle`) + `DA_Pistol` — Rider 빌드, VR Preview에서 잡기 → 발사·햅틱 → 탄 소진 Failed(햅틱 없음) → 놓으면 입력 해제 확인
  - Snap 잡기는 부모를 `NegateRotator(Grip 상대 회전)`으로 돌려 `Grip`의 축을 손 축에 맞춘다. 템플릿 `BP_Pistol` 값을 따라 `Grip` 회전 Pitch 85
  - 잦은 VR Preview 크래시(`MotionVectorIndex == MotionVectorDepthIndex`) 우회로 `DefaultEngine.ini`에 `xr.OpenXRAcquireMode=1` (3.6 참고, 효과는 더 지켜볼 것)
- [x] ✅ **Step 3 마일스톤**: VR Preview에서 권총을 집고 트리거로 발사

- [x] Step 4-1: `UEnemyDataAsset`(`MaxHealth`, `Armor`, `BoneMultipliers`(없는 본 1.0), `LegBones`, `LimpThreshold`, `GetBoneMultiplier`/`IsLegBone`), Third Person 콘텐츠 팩으로 전신 마네킹(`Characters/Mannequins`) 추가, `Portfolio/Data/DA_Enemy_Mannequin` — 커맨드라인 빌드, 에디터에서 분류·값 저장 확인
  - 맵 키는 `PA_Mannequin`에 바디가 있는 본만 쓴다: `pelvis`, `spine_02~05`, `neck_01`, `neck_02`, `head`, `clavicle_l/r`, `upperarm_l/r`, `lowerarm_l/r`, `hand_l/r`, `thigh_l/r`, `calf_l/r`, `foot_l/r`

- [x] Step 4-2: `IHittable`(`Weapon/Hittable.h`, C++ 전용 `UINTERFACE`), `UDamageReceiver`(`IHittable` 구현, `CalculateDamage`, `CurrentHealth`/`LegDamage`/`bLimping`/`bDead`, `OnDamaged`/`OnLimpStarted`/`OnDied`, `LogEnemy`), 투사체가 `FindComponentByInterface<IHittable>()`로 전달, Launch/Hit 로그에 위치 — 커맨드라인 빌드, `Portfolio/Enemies/BP_EnemyMannequin`(루트 `SKM_Manny_Simple`, `PhysicsActor`, `Damage Receiver`), 일반 Play에서 `thigh_r` 16 피해 누적 → 48/40에서 `Limp started` 한 번 확인
  - `DA_TestWeapon`의 `FireSequence`에서 `BP_GripStep`을 빼서 일반 Play `F`로 쏠 수 있게 함 (VR 권총 `DA_Pistol`은 그대로)
  - 레벨에 오래전 배치한 `BP_TestWeapon` 인스턴스의 `Muzzle`이 `Body`에 붙어 있지 않아 탄이 월드 (20, 0, 5)에서 나갔음 → 인스턴스를 다시 배치하고 레벨 블루프린트 참조를 다시 연결. **C++ 생성자의 컴포넌트 계층을 바꾸면 이미 배치된 인스턴스는 다시 배치한다.**
  - 스크린샷: Play 중 `F9` 또는 `HighResShot 1` (`Saved/Screenshots/WindowsEditor/`), 간단히는 `Shift+F1` → `Win+Shift+S`

- [x] Step 4-3: `UHitReaction`(`OnDamaged` 구독, 맞은 본 아래만 `SetAllBodiesBelowSimulatePhysics` + `PhysicalAnimation` 월드 기준 모터, `RecoverTime` 동안 블렌드 1 → 0, 반응 겹침은 `BoneIsChildOf`로 재시작·흡수, 반응 중에만 틱, `ApplyPhysicalHit`은 `BlueprintCallable`), `UAmmoDataAsset::ImpactImpulse`, `OnDamaged`에 `Ammo` 추가 — 커맨드라인 빌드, `BP_EnemyMannequin`에 `Physical Animation`·`Hit Reaction` 부착, 일반 Play에서 부위별 밀림과 복귀 확인
  - 골반(루트 바디)을 맞으면 몸이 돌아간 채 남던 문제: 메시 기본값 `SimulationUpatesComponentTransform`이 컴포넌트를 루트 바디에 따라가게 해서 월드 기준 목표 자세도 같이 움직였음 → `UHitReaction::BeginPlay`에서 `PhysicsTransformUpdateMode = ComponentTransformIsKinematic` (Live Coding으로 반영)
  - 움직임을 느리게 보려면 Play 중 콘솔 `slomo 0.2` (`slomo 1`로 복귀)

- [x] Step 4-4: `ELegSide`, 적 데이터 `LeftLegBones`/`RightLegBones`·`GetLegSide`, `OnLimpStarted(InjuredLeg)`, `UEnemyAnimInstance`(`NativeBeginPlay`에서 구독, `LimpAlpha`로 `PelvisOffset`·`Left/RightFootOffset` 계산) — 빌드, `Portfolio/Enemies/ABP_EnemyMannequin`(`MM_Idle` → `Transform (Modify) Bone` ×3(`pelvis`, `ik_foot_l`, `ik_foot_r`, `Add to Existing`) → `Leg IK`), `BP_EnemyMannequin` 메시에 연결, 일반 Play에서 오른쪽·왼쪽 다리 절뚝임 자세 확인
  - `Transform (Modify) Bone`은 디테일 패널에서 `Bone to Modify`와 `Translation Mode`만 바꾸면 된다 (나머지는 기본값이 `Ignore`/`Component Space`)
  - 포즈 핀은 로컬 → `Local To Component` → 컴포넌트 공간 노드들 → `Component To Local` → `Output Pose`로 한 줄이어야 한다

- [x] Step 4-5: `UHitReaction`이 `OnDied` 구독 → `StartRagdoll`(반응 비우고 틱 끔, `SetStrengthMultiplyer(0)`, 전신 물리, 이후 `ApplyPhysicalHit` 무시), `UEnemyDataAsset::CorpseLifeSpan`(기본 10초, 0이면 남김), `UDamageReceiver`가 사망 시 `SetLifeSpan` — 커맨드라인 빌드, 일반 Play에서 사망 → 래그돌 → 쓰러진 몸 피격 무시 → 제거 확인
- [x] ✅ **Step 4 마일스톤**: 마네킹의 부위별 피해와 물리 피격 반응, 절뚝임, 사망 래그돌 확인

### 현재 위치
- **Step 5** 시작 전 (`Docs/PLAN.md` Phase 3 확장성 증명을 스텝화)
- 나중에 정리: `FirstVREditor.Target.cs`에 `IncludeOrderVersion` 설정 (빌드 시 `[Upgrade]` 권고)
- 원격 저장소 없음

### 결정 기록
- 2026-10-29: 바이너리 에셋(`.uasset`, `.umap`, 원본 아트/미디어)은 Git LFS로 관리한다.
- 2026-10-29: Fab 에셋 폴더는 `Content/FPS_Assault_Pack/`, `Content/Nimikko_WesternTown/`. 저장소에서 제외하고, 새 Fab 에셋을 받으면 `.gitignore`에 폴더를 추가한다.
- 2026-10-29: `OnFailedFeedback`은 부모 `Report`(비가상)가 `Failed`일 때 자동 호출한다. 자식은 `Report(Failed)`만 부른다. 이유별 피드백이 필요해지면 이유 파라미터로 확장. (`Docs/PLAN.md` 4.4)
- 2026-10-29: 소스 주석(에디터 툴팁 포함)은 한글로 쓴다. 한글이 든 소스 파일은 UTF-8(BOM)으로 저장한다. 로그 문자열(`TEXT("...")`)은 영어.
- 2026-10-30: 탄창에는 한 종류의 탄만 들어간다(`LoadedAmmo` + `CurrentAmmo`). 배치 시 초기 탄 종류는 `AllowedAmmo`의 첫 항목. (`Docs/PLAN.md` 5.2)
- 2026-10-30: 투사체 클래스는 탄 데이터(`UAmmoDataAsset::ProjectileClass`)에 둔다. 발사 원점 `Muzzle`은 코어 생성자가 만든다. (`Docs/PLAN.md` 4장, 5.1)
- 2026-10-31: 잡기 자체는 VR 템플릿(`BP_GrabComponent`, `BP_XRPawn`)을 재사용한다. 파지 상태 원본은 `UGripStep::bHeld`(`SetHeld`로 갱신)이고, 템플릿과의 연결은 `BP_GripStep`이 `GripComponentName`으로 잡기 컴포넌트를 찾아 스스로 구독한다. (`Docs/PLAN.md` 4.7)
- 2026-10-31: 무기 루트는 코어가 만드는 박스 충돌체 `Body`(`PhysicsActor` 프로필)다. 템플릿 잡기는 잡기 컴포넌트의 부모만 손에 붙이므로 `Grip`은 `Body` 바로 아래에 둔다. 빈 `Root`에서는 `Muzzle`이 제자리에 남는 문제가 있었다. (`Docs/PLAN.md` 4장)
- 2026-10-31: 트리거 입력은 무기 쪽 입력 전용 컴포넌트 `UTriggerInputComponent`(발사 단계 아님)가 받아 `RequestFire`를 부른다. 잡히면 잡은 손의 템플릿 입력 매핑(`IMC_Weapon_*`, `IA_Shoot_*`)을 켜고 놓으면 끈다. 손 판별은 `BP_TriggerInput`이 `GetHeldByHand`로 한다. (`Docs/PLAN.md` 4.7)
- 2026-10-31: 권총은 `BP_PistolWeapon` + `DA_Pistol`로 새로 만들고 `BP_TestWeapon`은 실험용으로 남긴다. 기준점은 `Muzzle`·`Grip`·`MagazineWell`(자리만). 코어는 발사 시 `OnFired(Recoil)`만 발행하고, 발사 햅틱은 `UTriggerInputComponent`가 구독해서 준다. 시각 반동은 나중에. (`Docs/PLAN.md` 4.8)
- 2026-10-31: 적 데이터는 본마다 배율(`BoneMultipliers`, 없는 본은 1.0), 방어력은 비율(`min(1, Penetration / Armor)`, 계산은 `UDamageReceiver`), 절뚝임은 `LegBones`에 든 본의 누적 최종 피해(좌우 합산)가 `LimpThreshold` 이상일 때. `MaxHealth`도 적 데이터에 둔다. (`Docs/PLAN.md` 5.2, 6.4)
- 2026-11-01: 투사체는 인터페이스 `IHittable`(`Weapon/Hittable.h`, C++ 전용)로 피해를 넘긴다. 구현은 컴포넌트 `UDamageReceiver`가 하고, 투사체는 `FindComponentByInterface<IHittable>()`로 찾는다. `UDamageReceiver`는 `OnDamaged`/`OnLimpStarted`/`OnDied`를 발행만 하고 반응 쪽이 구독한다. 로그 카테고리 `LogEnemy`. (`Docs/PLAN.md` 4.6, 6.5)
- 2026-11-01: 피격 반응은 맞은 본 아래만 물리로 돌리고 `UPhysicalAnimationComponent`(월드 기준 모터)로 애니메이션 자세에 당긴 뒤 `RecoverTime` 동안 블렌드 가중치 1 → 0. 충격량은 탄의 `ImpactImpulse`(속도 변화 cm/s) × (최종 피해 / `BaseDamage`). `OnDamaged`에 `Ammo`를 추가했다. 컴포넌트가 루트 바디를 따라가지 않게 메시의 `PhysicsTransformUpdateMode`는 `ComponentTransformIsKinematic`. (`Docs/PLAN.md` 6.6)
- 2026-11-01: 절뚝임 자세는 절차적 보정(골반 내림 + 다친 발 IK 목표 올림 + `Leg IK`)이다. 다친 다리는 절뚝임을 시작시킨 탄이 맞은 쪽이고, 적 데이터 `LegBones`를 `LeftLegBones`/`RightLegBones`로 나눠 구분한다. `OnLimpStarted(InjuredLeg)`를 C++ 부모 `UEnemyAnimInstance`가 구독하고 오프셋을 계산해 애님 그래프에 넘긴다. 상태 머신 없이 `LimpAlpha` 블렌드. (`Docs/PLAN.md` 6.7)
- 2026-11-01: 사망은 모터를 끈 전신 래그돌이고 `UHitReaction`이 `OnDied`를 구독해 맡는다(메시 물리를 만지는 컴포넌트는 하나). 사망 후 피격은 무시. 쓰러진 몸은 적 데이터 `CorpseLifeSpan`(초, 0이면 남김) 뒤 제거하고, 수명은 `UDamageReceiver`가 사망 시 `SetLifeSpan`으로 건다. (`Docs/PLAN.md` 6.8)
