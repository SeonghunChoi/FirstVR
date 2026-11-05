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
  - **새 VR 입력 매핑(IMC)은 `프로젝트 세팅 > Enhanced Input > Default Mapping Contexts`에 등록해야** OpenXR 런타임이 그 버튼을 보내 준다(세션 시작 시 이 목록으로 액션을 만든다). 코드가 잡을 때 켜는 매핑이면 `Add Immediately`는 끈다.
  - 시뮬레이터 `B` 키 = 조작 중인 손의 A/X 버튼
  - 템플릿 메뉴·실패 화면 버튼은 레이저로 가리키고 시뮬레이터 마우스 왼쪽 클릭으로 누른다
  - 잡기 토글: `BP_XRPawn`의 `bToggleGrab`(기본값)을 켜면 `U`를 한 번 눌러 잡고 한 번 더 눌러 놓는다. 실기기에서는 끈다
  - 시험용 콘솔 명령(프리뷰 창에서 `~`): `Enemy.Limp [Left|Right]` 절뚝임, `Enemy.Kill` 사망
  - 두 손으로 동시에 잡으려면 `]`/`[`로 두 손을 함께 선택해야 하고, 이때 `U`를 떼면 양손이 같이 풀린다 (한 손만 놓는 동작은 시뮬레이터로 시험할 수 없다)
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

### Step 5. 확장성 증명 (`Docs/PLAN.md` Phase 3)
- 5-1. `USafetyStep`: 잠금 상태, 잠겨 있으면 Failed + 피드백, `ToggleSafety`/`SetLocked`, 배치별 초기 잠금 → 일반 Play에서 키로 토글
- 5-2. 입력 부모 `UWeaponInputComponent` 분리(트리거 리팩터링) + `USafetyInputComponent`(잡은 손 A/X) → VR Preview
- 5-3. 두 손 파지 `UTwoHandGripStep` (설계 판단 완료, `Docs/PLAN.md` 4.10)
  - 5-3a. 보조 손잡이(붙이지 않는 잡기) 상태 + 한 손/두 손 `RecoilScale` 기록, 항상 Success → VR Preview에서 `FIRE` 로그·햅틱 세기
  - 5-3b. 두 손 조준: 두 손일 때만 틱, 총구를 주 손잡이 → 보조 손 방향으로
  - 5-3c. 주 손을 놓으면 보조 손이 이어받기, 보조 손까지 놓으면 떨어짐
- 5-4. 소총 (발사 모드 설계 판단 완료, `Docs/PLAN.md` 4.11)
  - 5-4a. 단발 소총: 새 탄 데이터 + `DA_Rifle` + `BP_RifleWeapon`, 새 C++ 없이 기존 컴포넌트 재조합
  - 5-4b. `UTriggerInputComponent` 발사 모드: 쓸 수 있는 모드(설정값) + 현재 모드(상태), 연발이면 누르는 동안 반복 요청
- 5-5. 빈 무기에 컴포넌트를 붙여 가는 시나리오 정리 (`Docs/WEAPON_ASSEMBLY.md`, 스크린샷은 `Docs/Images/Assembly/`)
  - 5-5a. 조합표: 기존 권총·단발 소총·연발 소총의 컴포넌트 패널과 데이터 에셋 스크린샷, 조합표와 흐름 한 줄
  - 5-5b. 빈 무기에서 다시 조립: `AWeaponBase` 빈 BP에 파지 → 탄창 → 안전장치 → 두 손 → 조정간 순서로 붙이며 단계마다 VR Preview 로그·스크린샷, 결과는 연발 소총과 같은 조합
- ✅ 마일스톤: `소총 = 파지 → 안전장치 → 탄창 → 발사`를 추가 C++ 없이 조립

### Step 6. 전투 구간 (`Docs/PLAN.md` Phase 4, 설계 6.9)
- 6-1. `AEnemyCharacter`(C++, `ACharacter`): 캡슐 루트, 메시 아래로, 기존 적 컴포넌트를 생성자에서, 사망 시 캡슐·이동 정지 → 새 적 BP로 일반 Play에서 부위 피해·절뚝임·래그돌 회귀
- 6-2. 걷기: 애님 블루프린트에 `BS_Idle_Walk_Run`(속도) + 절뚝임 보정 유지, 적 데이터 이동 속도·절뚝일 때 배율
- 6-3. State Tree 추적: 내비메시, AI 컨트롤러, 플레이어를 향한 MoveTo → VR Preview에서 다가옴
- 6-4. 공격: 공격 거리 상태, `MM_Attack_*` 몽타주, Anim Notify 타격 순간 주먹 주변 ↔ HMD 판정, `ApplyDamage`, 적 데이터 공격 값
- 6-5. 플레이어 체력 (설계 판단 완료, `Docs/PLAN.md` 6.9)
  - 6-5a. `UPlayerHealth`(`OnTakeAnyDamage` 구독, `OnHealthChanged`/`OnDied`) + C++ 게임 모드 뼈대(`SetPlayerDefaults`에서 구독, 실패 상태) + 콘솔 명령 → 로그
  - 6-5b. `UPlayerHitFeedback`: 화면 붉게 번쩍임, 양손 진동, 피격음
  - 6-5c. 왼손 손목 체력 위젯
  - 6-5d. 실패 화면(3D 위젯) + 위젯 포인터로 '다시 시작' → 레벨 다시 열기
- 6-6. 웨이브: 웨이브 데이터 에셋 + 스폰 지점, 전멸 시 다음 웨이브, 마지막이면 승리
  - 6-6a. 웨이브 진행: `UWaveDataAsset` + `AEnemySpawnPoint`(이름으로 고름) + 게임 모드 컴포넌트 `UWaveSpawner`, 모두 막으면 `Victory` → 로그와 `Enemy.Kill`로 확인
  - 6-6b. 승리 화면: `VictoryScreenClass` + BP·WBP (부모 위젯은 실패 화면과 공유)
- 6-7. 탄창 교체: 집는 탄창 액터, `MagazineWell`에 끼워 장전 (세부 설계 판단은 이 스텝에서)
- 6-8. 전투 레벨 구성: 무기·탄창 배치, 시작과 끝
- 6-9. 플레이 테스트와 데이터 튜닝
- 6-10. 실기기 검증 (헤드셋 확보 시, 시뮬레이터 런타임 토글 해제)
- ✅ 마일스톤: VR Preview에서 무기를 집어 웨이브를 막아 내거나 맞아서 실패하는 한 구간

### Step 7 이후
- `Docs/PLAN.md` Phase 5(PDF 제작)

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

- [x] Step 5-1: `USafetyStep`(`bLocked`, `EditInstanceOnly` `bInitiallyLocked`=true, 잠겨 있으면 Failed, `ToggleSafety`/`SetLocked`/`IsLocked`, 바뀔 때만 `OnSafetyChanged`) — 커맨드라인 빌드, `BP_SafetyStep`(실패·변경 피드백 Print String), `BP_TestWeapon`에 부착, `DA_TestWeapon` `FireSequence`=[`BP_SafetyStep`, `MagazineStep`], 레벨 블루프린트 `G` → `ToggleSafety`. 일반 Play에서 잠금 시 1/2단계 중단, 풀면 발사 확인. 코어·탄창 코드 변경 없음

- [x] Step 5-2: 추상 부모 `UWeaponInputComponent`(`EnableForHand`/`DisableForHand`, `IsHandInputEnabled`, `FindGripComponent`, 손별 `Left/RightContext`·`Left/RightAction`, 자식은 `HandleAction`만 재정의, 켜진 입력 컴포넌트가 없을 때만 액터 입력 끔), `UTriggerInputComponent`는 `RequestFire`·햅틱만, `USafetyInputComponent`(같은 액터 `USafetyStep::ToggleSafety`), 바뀐 이름은 `DefaultEngine.ini` `[CoreRedirects]`로 연결 — 빌드, `Portfolio/Input/IA_Safety_*`·`IMC_Safety_*`(A/X 누름), `BP_SafetyInput`(`BP_TriggerInput` 그래프 복제), `BP_TestWeapon`에 부착, VR Preview에서 잡기 → `B`로 풀기 → 발사·햅틱 → 다시 잠금 → `Safety on` 중단 → 놓으면 마지막 하나만 `actor input off` 확인
  - 처음엔 A/X가 무반응: 새 IMC가 `Default Mapping Contexts`에 없어 OpenXR 액션으로 등록되지 않았음 → 등록(`Add Immediately` 끔)으로 해결 (3.6)

- [x] Step 5-3a: `UTwoHandGripStep`(추상, 항상 Success, `SetMainHand`/`SetSupportHand`(손 `USceneComponent`, 놓이면 None), `IsTwoHanded`, `One/TwoHandRecoilScale`=1.0/0.4를 `RecoilScale`에 곱함, `GripComponentName`="Grip"·`SupportComponentName`="SupportGrip") — 빌드, `BP_TwoHandGripStep`(BeginPlay에서 두 잡기 컴포넌트의 `OnGrabbed`/`OnDropped` 구독 → `MotionControllerRef` 전달), `BP_TestWeapon`에 `SupportGrip`(`BP_GrabComponent`, Custom) 추가, `DA_TestWeapon` `FireSequence`=[`BP_TwoHandGripStep`, `BP_SafetyStep`, `MagazineStep`]. VR Preview에서 한 손 x1.00, 두 손 x0.40 → `FIRE` `Recoil 0.40`·햅틱 0.40, 보조 손만 잡아도 무기는 붙지 않음 확인
  - 블루프린트에서 다른 컴포넌트의 변수는 그 컴포넌트 변수 핀에서 GET으로 꺼낸다 (셀프 대상 SET 노드를 놓으면 `Target` 연결 오류)

- [x] Step 5-3b: `UTwoHandGripStep` 두 손 조준 — 두 손일 때만 틱(`TG_PostPhysics`), 조준 시작 시 무기 루트의 부모(주 손)·상대 트랜스폼·두 손잡이 액터 기준 위치 저장, 매 틱 저장한 한 손 자세에서 `FindBetweenNormals`로 주 손잡이를 축 삼아 보조 손을 향하게 회전, 보조 손을 놓으면 한 손 자세 복원(주 손을 놓아 이미 떨어졌으면 복원 안 함) — Rider 빌드, VR Preview에서 `Two-hand aim on/off`, 회전과 복원 확인
  - 두 손 상태에서 주 손을 놓으면 무기는 떨어지지만 보조 손의 Custom 잡기는 `Support held`로 남는다 → 5-3c에서 처리

- [x] Step 5-3c: `UTwoHandGripStep` 보조 손 들기 — `UpdateSupportHold`가 "보조 손만 잡음"(`SupportHand && !MainHand`) 변화를 보고 `AttachToSupportHand`(물리 끄고 `KeepWorldTransform`으로 붙임) / 주 손이 다시 잡으면 `Main hand took over` / `DropFromSupportHand`(떼고 물리 켬), 상태 `bHeldBySupport` — Rider 빌드, VR Preview에서 보조 손잡이만 잡아 들고 옮기기·떨어뜨리기, 보조 손에 들린 무기를 주 손이 잡아 두 손 조준 복귀 확인
  - **두 손 상태에서 주 손만 놓는 이어받기는 미검증**: 시뮬레이터는 두 손을 함께 선택해야 동시에 잡을 수 있고 `U`를 떼면 양손이 같이 풀린다. 같은 붙이기 경로(`AttachToSupportHand`)를 쓰므로 동작할 것으로 보고, 헤드셋이 생기면 확인한다

- [x] Step 5-4a: 단발 소총을 새 C++ 없이 조립 — `DA_Ammo_556`(`Speed` 90000, `Penetration` 3, `BaseDamage` 35, `ImpactImpulse` 800, 투사체는 `BP_Projectile_9mm` 재사용), `DA_Rifle`(`FireSequence`=[`BP_GripStep`, `BP_TwoHandGripStep`, `BP_SafetyStep`, `BP_MagazineStep`], `Recoil` 1.5, `FireRate` 6), `BP_RifleWeapon`(`BP_PistolWeapon` 복제 + `SupportGrip`(Custom)·두 손 파지·안전장치·안전장치 입력, 탄창 30/`DA_Ammo_556`) — VR Preview에서 잠금 시 3/4단계 중단, 풀면 `FIRE (DA_Ammo_556, Recoil 1.50)`·햅틱 1.00, 머리 명중 105 피해로 사망 확인. 권총 에셋은 변경 없음
  - 상속(자식 블루프린트)이 아니라 복제: 소총은 권총의 한 종류가 아니라 다른 조합이다

- [x] Step 5-4b: `EFireMode`(`Single`/`Auto`), 코어 `CanRequestFire()`·`OnFireAborted`, 입력 부모에 `HandleActionReleased`(Completed 바인딩)·매핑 우선순위 1, `UTriggerInputComponent` 발사 모드(`FireModes` 설정값, `FireMode` 상태, 연발 중에만 틱으로 `CanRequestFire`일 때 요청, 놓거나 Failed면 멈춤), `USelectorInputComponent`(A/X로 안전 → `FireModes` 순서 순환) — 커맨드라인 빌드, `Portfolio/Input/IA_Fire_*`·`IMC_Fire_*`(트리거 없음, 키 `Oculus Touch (L/R) 트리거`, `Default Mapping Contexts` 등록), `BP_TriggerInput`을 `IMC_Fire_*`/`IA_Fire_*`로 교체, `BP_SelectorInput`(`BP_SafetyInput` 복제 후 부모 변경), `BP_AutoRifleWeapon`(`BP_RifleWeapon` 복제, 안전장치 입력 → 조정간, `FireModes`=[`Single`, `Auto`]) — VR Preview에서 `B`로 안전 → 단발 → 연발, 단발은 당길 때 한 발, 연발은 약 0.17초 간격(`FireRate` 6) 반복 후 놓으면 `trigger released`, 빈 탄창에서는 한 번 `Empty` 후 `fire aborted`로 멈춤 확인
  - 미확인: 권총·단발 소총 회귀 (같은 코드 경로라 위험은 낮음). 연발 → 안전 순환은 5-5b에서 확인
  - 출력 로그의 `IMC_* has a Description, "" ... Identical localized descriptions` 경고는 IMC의 `Description`을 비워 둬서다. 이름으로 대체되어 동작에는 문제없고, IMC마다 `Description`을 채우면 사라진다

- [x] Step 5-5a: `Docs/WEAPON_ASSEMBLY.md`(조립 규칙, 권총·단발 소총·연발 소총 조합표와 흐름 한 줄, 무기마다 늘어난 C++, 5-5b 계획), 스크린샷 6장 `Docs/Images/Assembly/`(세 무기 `Components` 패널, `DA_Pistol`·`DA_Rifle`, 연발 소총 `Fire Modes`) — 스크린샷과 표 대조, `Recoil`/`FireRate`(권총 1.0/3, 소총 1.5/6)와 외형 행 추가

- [x] Step 5-5b: 빈 `AWeaponBase` BP에서 다시 조립 — `Portfolio/Weapons/BP_AssembledRifle` + `Portfolio/Data/DA_AssembledRifle`, 단계 0(빈 무기 `WeaponData is not set`) → 1(잡기·트리거, `No ammo in context`) → 2(탄창, 3발 후 `Fire aborted at step 2/2`) → 3(안전장치, 데이터만 먼저 넣어 `No component for step` 확인) → 4(두 손, `Recoil` 1.50 → 0.60) → 5(조정간·`FireModes`=[`Single`, `Auto`], 데이터 변경 없음, 연발 반복·떼면 멈춤·안전에서 `fire aborted`·빈 탄창 멈춤), 단계마다 VR Preview 로그와 스크린샷 `Docs/Images/Assembly/step*_*.png`, `Docs/WEAPON_ASSEMBLY.md` 4장에 결과와 연발 소총 대조표. C++ 빌드 없음
  - `BP_AutoRifleWeapon` 외형 `Shoulder` → `Gunstock` 이름 변경, `autorifle_components.png` 다시 촬영
  - 재조립 무기에는 `MagazineWell`(쓰는 시스템이 없는 자리 표시)을 넣지 않았다
- [x] ✅ **Step 5 마일스톤**: `소총 = 파지 → 안전장치 → 탄창 → 발사`를 추가 C++ 없이 조립 (단발 소총 5-4a, 빈 무기에서 연발 소총 재조립 5-5b)

- [x] Step 6 설계 판단과 스텝 나누기 (`Docs/PLAN.md` 6.9)
- [x] Step 6-1: `AEnemyCharacter`(추상, `ACharacter`, 생성자에서 `DamageReceiver`·`PhysicalAnimation`·`HitReaction`, 메시 충돌 `Ragdoll` 프로필, `OnDied` 구독 → 캡슐 충돌 끔·이동 정지), 투사체가 `Pawn` 채널 무시(캡슐을 지나 메시 바디에 맞음) — 커맨드라인 빌드, `Portfolio/Enemies/BP_EnemyManny`(`SKM_Manny_Simple`, `ABP_EnemyMannequin`, 메시 Z -90·Yaw -90, `DA_Enemy_Mannequin`), 일반 Play에서 `thigh_r` 누적 48/40 `Limp started (Right leg)`, 머리 60 ×2 → `Died` → `Ragdoll` → `Body stopped` 확인. 기존 적 컴포넌트 코드 변경 없음
  - 콘텐츠 브라우저에서 스켈레탈 메시를 끌어 놓으면 블루프린트가 아니라 `SkeletalMeshActor`가 생긴다. 적은 블루프린트(`BP_EnemyManny`)를 끌어 놓는다
  - 옛 `BP_EnemyMannequin`(메시 루트)은 에디터에서 삭제(레벨 인스턴스 포함)
- [x] Step 6-2: 적 데이터 `WalkSpeed`(150 cm/s)·`LimpSpeedScale`(0.5), `AEnemyCharacter`가 `BeginPlay`와 `OnLimpStarted`에서 `MaxWalkSpeed` 갱신, `UDamageReceiver::GetEnemyData`, `UEnemyAnimInstance::GroundSpeed`(폰 수평 속도) — 커맨드라인 빌드, `ABP_EnemyMannequin`의 `MM_Idle`을 `BS_Idle_Walk_Run` 블렌드 스페이스 플레이어(`Ground Speed`)로 교체, 레벨에 내비 메시 바운드 볼륨, 레벨 블루프린트 `M` → `AI MoveTo`(플레이어 폰) — 일반 Play에서 `Walk speed 150 (x1.00)`, 다가오는 중 `thigh_l` 48/40 → `Limp started (Left leg)` → `Walk speed 75 (x0.50)` 확인
  - 상속받은 C++ 변수는 `내 블루프린트 (My Blueprint)`에서 `상속된 변수 표시 (Show Inherited Variables)`를 켜야 보인다. 그래프 우클릭 검색(`Ground Speed`)은 설정과 상관없이 나온다
  - 출력 로그의 `LogCrowdFollowing: Unable to find RecastNavMesh ... UCrowdManager` 경고는 Play 종료(`BeginTearingDown`) 때만 나온다. 이동에는 영향 없음
- [x] Step 6-3: `StateTree`·`GameplayStateTree` 플러그인, `Build.cs`에 `AIModule`·`StateTreeModule`·`GameplayStateTreeModule`, 적 데이터 `AttackRange`(100 cm), `AEnemyAIController`(`StateTreeAI` 컴포넌트, 자동 시작 끄고 폰을 잡고 게임이 시작된 뒤 `StartLogic`, 폰을 놓으면 `StopLogic`), 글로벌 작업 `FEnemyTargetTask`(머리 아래 바닥 `TargetLocation`·수평 `Distance`·`AttackRange`·`bInRange`, 들어올 때 +10 / 나갈 때 +`RangeHysteresis` 50), `AEnemyCharacter`가 기본 AI 컨트롤러·`PlacedInWorldOrSpawned`, 사망 시 `DetachFromControllerPendingDestroy` — 커맨드라인 빌드, `Portfolio/Enemies/BP_EnemyAIController`(`StateTreeAI`=`ST_Enemy`), `ST_Enemy`(스키마 `StateTree AI Component`, 글로벌 작업 `Enemy Target`, `Chase`(조건 `bInRange = False`, `다음으로 이동` 대상 ← `TargetLocation`·허용 가능 반경 ← `AttackRange`·대상 이동 허용치 30·반경 포함 둘 다 끔, 상태 완료 시 → `Root`), `Hold`(`딜레이 작업` 영구 실행, `On Tick` + `bInRange = False` → `Chase`)), `BP_EnemyManny` AI 컨트롤러 클래스 지정, 레벨 블루프린트 `M` 키 제거 — 일반 Play에서 1500 cm를 9.2초에 걸어와 `Target in range (109)` → `Chase` → `Hold` 한 번 전환, VR Preview에서 머리를 옮기면 `out of range (152)` → `Chase` → `in range (109)` → `Hold`, 머리 명중 사망 → `Body stopped` → `StopLogic ... 'Unpossessed'` → `State Tree stopped` 확인
  - 상태 전환을 보려면 Play 중 콘솔 `log LogStateTree Verbose` (Play를 끝내면 꺼진다). `ST_Enemy` 에디터를 열어 두면 활성 상태가 강조된다
  - `Target tracking started`는 컨트롤러의 `State Tree started`보다 먼저 찍힌다(`StartLogic` 안에서 글로벌 작업이 먼저 시작)
- [x] Step 6-4: 적 공격 — `UEnemyAttack`(머리 쪽으로 돈 뒤 몽타주, 판정 구간 `Begin/Update/EndStrikeWindow`로 본이 지나간 선분 ↔ 몸 캡슐, 구간당 한 번 `ApplyDamage`, 사망 시 취소), Anim Notify `Enemy Strike`·Anim Notify State `Enemy Strike Window`(`StrikeBone`, `Radius` 15), State Tree 조건 `Enemy Can Attack`·태스크 `Enemy Attack`, 적 데이터 `AttackMontages`·`AttackDamage`·`AttackCooldown`, `UPlayerBody`(`Player/`, HMD를 따라가는 `Pawn` 캡슐, 바닥은 아래로 트레이스, 스폰 중엔 충돌 끔), `UEnemyAnimInstance::LegIKAlpha`, 시험용 `UDamageReceiver::StartLimp`/`Kill`과 콘솔 명령 `Enemy.Limp [Left|Right]`/`Enemy.Kill` — 빌드, `Portfolio/Enemies/Attack/AM_Attack_01~03`(판정 구간 `hand_r`/`hand_l`/`foot_r`), `ST_Enemy` 공격 상태, `ABP_EnemyMannequin` 슬롯·`Leg IK` Alpha, `BP_XRPawn`에 `PlayerBody`와 잡기 토글(`bToggleGrab`), `PlayerStart` Z 0 — VR Preview에서 세 공격 명중(20 피해), 뒤로 물러나면 `missed`, 절뚝여도 발차기 발이 올라감, 공격 중 `Enemy.Kill` → `Attack interrupted` 확인
  - VR 폰은 캡슐이 없고 루트가 바닥(트래킹 원점)이다. `PlayerStart`는 캐릭터 캡슐 중심 높이(Z 93)에 놓이므로 VR 레벨에서는 **Z를 바닥(0)으로** 내린다
  - 레벨 바닥 `Floor`는 `BlockAllDynamic`(오브젝트 타입 `WorldDynamic`)이라 바닥 트레이스는 `WorldStatic`과 `WorldDynamic`을 함께 찾는다
  - 시뮬레이터 이동 속도는 `%APPDATA%\MetaXR\MetaXrSimulator\persistent_data.json`의 `movement_speed`(지금 5.0)
  - `bToggleGrab`은 시뮬레이터 시험용 모드 스위치다(누를 때마다 잡기/놓기, 놓음 이벤트 무시). 실기기에서는 끈다

- [x] 레이저 사이트(Step 6-4 뒤 추가, 시뮬레이터 조준 보조): `ULaserSightComponent`(추상, 잡았을 때만 `SetLaserEnabled`, `Muzzle` 아래 런타임 `UNiagaraComponent`, 매 틱 `WorldStatic`·`WorldDynamic`·`PhysicsBody` 트레이스(무기 자신·날아가는 탄 무시), `SetVariablePosition`으로 `BeamStart`/`BeamEnd`, 최대 3000cm), `Build.cs`에 `Niagara` — 빌드, `Portfolio/Weapons/FX/NS_LaserBeam`(`DynamicBeam` 템플릿, User Parameter `BeamStart`/`BeamEnd`(Position) → `Beam Emitter Setup`(Absolute), `Scale Curve` 0.5, `Emitter State` `Once` + `Infinite`, 수명 끝나도 파티클 유지), `Portfolio/Weapons/Steps/BP_LaserSight`(잡기 컴포넌트 `OnGrabbed`/`OnDropped` 구독), `BP_PistolWeapon`에 부착 — VR Preview에서 잡으면 `Laser on`·놓으면 `Laser off`, 빔이 맞은 곳에서 끊기고 끊김 없이 유지 확인
  - 나이아가라 템플릿 `DynamicBeam`은 루프마다 빔을 새로 만들어 주기적으로 꺼진다. 계속 켜 둘 빔은 한 번만 생성하고 파티클이 죽지 않게 한다
  - 놓으면 `Laser off`로 꺼짐 확인. 소총들에는 아직 붙이지 않음

- [x] Step 6-5a: `UPlayerHealth`(`Player/`, 폰 `OnTakeAnyDamage` 구독, `MaxHealth` 100, `TakeHealthDamage`, 시작할 때 한 번·바뀔 때마다 `OnHealthChanged(Current, Max, Damage)`, 0이면 `OnDied`, 죽은 뒤 피해 무시, `LogPlayer`를 여기로 옮김), `AFirstVRGameMode`(`Game/`, `SetPlayerDefaults`에서 `OnDied` 구독 → `EEncounterState::Failed`, `OnEncounterStateChanged`, `RestartEncounter`는 현재 레벨 다시 열기, `LogEncounter`), 콘솔 명령 `Player.Damage [Amount]`·`Encounter.Restart`, `UPlayerBody`는 스폰 중 충돌을 끄고 BeginPlay에서 켬 — 빌드, `Portfolio/Game/BP_FirstVRGameMode`(`L_WeaponTest` 월드 세팅 게임 모드 오버라이드), `BP_XRPawn`에 `Player Health` 부착 — VR Preview에서 `Watching ... health` → `Health 100/100`, 적 공격 20 ×5 → `Player died` → `Encounter Failed`, `Encounter.Restart` → `Restarting L_WeaponTest` → 다시 `Watching`·`Health 100/100` 확인
  - 일반 Play는 헤드셋이 없어 카메라가 폰 원점(바닥, `PlayerStart` Z 0)에 있고 몸 캡슐이 50cm라 적 공격 시험은 VR Preview에서 한다
  - 죽은 뒤에도 적은 계속 공격한다. 실패 시 적을 멈추는 것은 6-5d/6-6에서

- [x] Step 6-5b: `UPlayerHitFeedback`(`Player/`, `OnHealthChanged` 피해 > 0 구독, `FlashMaterial`의 MID를 카메라 `AddOrUpdateBlendable`로 넣고 `Intensity`를 `FlashDuration` 0.4초 동안 1 → 0(이때만 틱, 쉴 때 가중치 0), 양손 `PlayHapticEffect`, `PlaySound2D`(`HitSoundVolume`/`HitSoundPitch`)) — 빌드, `Portfolio/Player/M_HitVignette`(도메인 Post Process, `PostProcessInput0` RGB와 `Color`(RGB)를 `ScreenPosition` `ViewportUV` → `RadialGradientExponential` → `1-x` × `Intensity` → `Saturate`로 `Lerp` → 이미시브 컬러), `BP_XRPawn`에 `Player Hit Feedback`(`PistolFireHapticEffect`, `Fire_Cue` 피치 0.5) — VR Preview에서 `Hit feedback ready`, 적 공격·`Player.Damage`마다 `Hit feedback`, 가장자리 붉게 번쩍임과 피격음 확인
  - 머티리얼 `Lerp`는 A·B 채널 수가 같아야 한다. 벡터 파라미터는 `RGBA`가 아니라 `RGB` 핀을 쓴다. `Lerp` 노드는 검색에서 `LinearInterpolate`(단축키 `L`+클릭)
  - `Fire_Cue`는 one-shot이 아니라 `PlaySound2D` 때 `orphaned sound` 경고가 나온다. 임시 소리라 그대로 두고, 짧은 사운드 웨이브로 교체하면 사라질 것
  - 진동은 시뮬레이터에서 느낄 수 없어 미확인

- [x] Step 6-5c: `UPlayerHealthWidget`(`UI/`, 추상 `UUserWidget`, `BindWidget` `HealthBar`·`BindWidgetOptional` `HealthText`, `NativeConstruct`에서 `GetOwningPlayerPawn`의 `UPlayerHealth` 구독(디자인 타임 제외)·구독 즉시 현재 값 표시, `NativeDestruct`에서 해제, `BindToHealth`, `OnHealthDisplayUpdated`), `Build.cs`에 `UMG`·`Slate`/`SlateCore` — 빌드, `Portfolio/UI/WBP_WristHealth`(Vertical Box > `HealthBar`·`HealthText`), `BP_XRPawn` `MotionControllerLeftGrip` 아래 Widget `WristHealth`(`Space` World, `Draw Size` 200×60, 스케일 0.05, `NoCollision`, 뷰포트에서 손등에 맞춤) — VR Preview에서 `Health widget bound`, 손목을 따라 움직이고 맞을 때마다 막대·글자 갱신 확인
  - 손 메시는 컨트롤러 아래에서 따로 회전돼 있어 컨트롤러 축으로 짐작한 위치는 손목 옆이 됐다. 손에 붙이는 것은 `BP_XRPawn` 뷰포트에서 손 메시를 보며 맞춘다
  - 적을 잠깐 멈추려면 레벨 인스턴스의 `Auto Possess AI`를 `Disabled`로(컨트롤러가 없어 State Tree가 돌지 않음). 언리얼 액터에는 `SetActive` 같은 단일 스위치가 없다
  - `Encounter.Restart` 뒤 표시는 직접 보지 않음 (새 Play마다 `bound`·`100/100`으로 시작하는 것은 확인, 다시 열기도 같은 경로)

- [x] Step 6-5d: `UFailScreenWidget`(`UI/`, 추상, `BindWidget` `RestartButton` `OnClicked` → 한 번만 누르게 끄고 게임 모드 `RestartEncounter`), 게임 모드 `FailScreenClass`·`FailScreenDistance`(100) → 실패 시 `HaltEnemies`(`TActorIterator<AEnemyAIController>`) + 머리 앞 수평 방향에 스폰, `AEnemyAIController::HaltLogic`(State Tree `StopLogic` + `StopMovement`, `bHalted`면 다시 시작 안 함) — 빌드, `Portfolio/UI/WBP_FailScreen`(`FAILED` + `RestartButton`/`Restart`), `Portfolio/UI/BP_FailScreen`(템플릿 `BP_Menu` 복제, 위젯 클래스만 교체), `BP_FirstVRGameMode` `Fail Screen Class` — VR Preview에서 `Player died` → `Encounter Failed` → `Attack interrupted` → `State Tree halted` → `Halted 1 enemies` → `Fail screen ... spawned`, 레이저로 `Restart`를 마우스 왼쪽 클릭 → `Restart clicked` → `Restarting L_WeaponTest` → `Health 100/100`, 다시 시작한 판에서도 같은 흐름 확인
  - WBP에 `BindWidget` 이름의 위젯이 없으면 `"RestartButton" 필수 위젯 바인딩을 찾지 못했습니다` 컴파일 오류가 난다(버튼 이름을 맞추면 사라짐)
- [x] ✅ **Step 6-5**: 플레이어 체력·피격 피드백·손목 체력·실패 화면과 다시 시작

- [x] Step 6-6a: `UWaveDataAsset`(`Game/`, `StartDelay`, `Waves` = `FWaveDefinition`(`Groups`, `RestAfterClear`), 묶음 `FWaveSpawnGroup`(`EnemyClass`·`Count`·`SpawnPointName`·`StartDelay`·`SpawnInterval`, 0이면 한꺼번에)), `AEnemySpawnPoint`(바닥 위치·에디터 전용 화살표·`EditInstanceOnly` `SpawnPointName`), 게임 모드 컴포넌트 `UWaveSpawner`(묶음마다 반복 타이머, CDO 캡슐 반 높이만큼 올려 스폰, 스폰한 적 `OnDied` 구독으로 생존 수, 다 나오고 0명이면 `OnWaveCleared` → `RestAfterClear` 뒤 다음, 마지막이면 `OnAllWavesCleared`, 스폰 실패도 남은 수를 줄임, `StopWaves`, `LogWave`), `EEncounterState::Victory`, 게임 모드 `BeginPlay`에서 `StartWaves`, 모두 막으면 `Victory`, 실패 시 `StopWaves`, 결과가 난 뒤의 사망·승리 무시 — 빌드, 레벨 배치 적 삭제, 스폰 지점 `Front`·`Left`, `Portfolio/Data/DA_Waves_Test`(웨이브 1: `Front` 1명, 웨이브 2: `Front` 2명 간격 3 + `Left` 1명 지연 2), `BP_FirstVRGameMode` `Wave Spawner` `Wave Data` — 일반 Play에서 웨이브 1 → `Enemy.Kill` → 5초 쉼 → 웨이브 2(묶음 지연·간격대로) → `Enemy.Kill` → `All waves cleared` → `Encounter Victory`, 웨이브 2 도중 `Player.Damage 100` → `Encounter Failed` → `Waves stopped at wave 2/2` → `Halted 3 enemies` → 실패 화면 확인
  - `Spawn Point Name`은 아웃라이너 이름(액터 라벨)과 다르다. 라벨은 에디터 전용 값이라 게임에서 쓰지 않는다. 비워 두면 `Spawn point 'X' not found`
  - 실패 시험은 웨이브 2의 적이 다 나온 뒤였다. 스폰 도중 실패해 남은 스폰이 취소되는 것은 직접 보지 않음(`ClearAllTimersForObject` 경로)

### 현재 위치
- **Step 6-6b** 다음 (승리 화면: `VictoryScreenClass` + BP·WBP, 부모 위젯 공유)
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
- 2026-11-02: Step 5 순서는 안전장치 → 두 손 파지 → 소총 → 정리. 안전장치는 발사 단계 `USafetyStep`(잠기면 Failed, 배치별 초기값 `bInitiallyLocked`)이고, VR에서는 잡은 손의 A/X로 토글한다. 버튼은 입력 컴포넌트가 받으며, 트리거와 공통부분을 추상 부모 `UWeaponInputComponent`로 꺼낸다. 손 판별은 입력 BP 자식마다 잡기 컴포넌트를 구독한다. (`Docs/PLAN.md` 4.9)
- 2026-11-02: 두 손 파지는 발사 조건이 아니다. `UTwoHandGripStep`은 항상 Success이고 한 손/두 손 `RecoilScale`을 기록한다. 보조 손을 잡으면 이 단계가 직접 틱으로 두 손 조준을 하고, 두 손 상태에서 주 손을 놓으면 보조 손이 무기를 이어받는다. 보조 손잡이는 무기를 손에 붙이지 않는 템플릿 잡기다. 5-3a/b/c로 나눠 진행. (`Docs/PLAN.md` 4.10)
- 2026-11-02: 주 손 없이 보조 손잡이만 잡아도 무기가 보조 손에 들린다(이어받기와 같은 "보조 손만 잡음" 상태, 붙이기·내려놓기는 `UTwoHandGripStep`이 직접). 보조 손만으로는 쏠 수 없다. (`Docs/PLAN.md` 4.10)
- 2026-11-02: 연발은 발사 단계가 아니라 트리거 입력 컴포넌트가 맡는다(누르는 동안 반복 요청, 속도는 코어 `FireRate`). 쓸 수 있는 모드는 트리거 설정값, 현재 모드는 상태값. 단계는 약실 상태(볼트액션) 같은 "이번 한 발" 조건에 쓴다. 단발 소총(5-4a) → 발사 모드(5-4b). (`Docs/PLAN.md` 4.11)
- 2026-11-02: 조정간은 새 컴포넌트 `USelectorInputComponent`가 A/X로 `안전 → 트리거 FireModes 순서`를 돌린다(안전장치 입력은 유지, 연발 소총만 교체). 연발 중 Failed면 반복을 멈추고 다시 당겨야 한다(코어 `OnFireAborted`). 트리거 입력은 놓음(`Completed`)을 받기 위해 템플릿 대신 트리거 없는 `IA_Fire_*`/`IMC_Fire_*`를 쓴다. (`Docs/PLAN.md` 4.11)
- 2026-11-03: 5-5 결과물은 조합표(기존 무기 3종)와 빈 무기에서 다시 조립하는 따라하기 둘 다다. 다시 조립할 무기는 기존 연발 소총과 같은 조합이다. 연발은 5-4b에서 C++을 추가한 뒤 조립했으므로, 문서에는 무기마다 그때 늘어난 C++을 그대로 적는다. (`Docs/PLAN.md` 9.2)
- 2026-11-03: 전투 구간은 적이 다가와 공격하고 플레이어 체력 0이면 실패, 웨이브 데이터를 다 막으면 승리. 적 판단은 State Tree, 공격 판정은 타격 순간(Anim Notify) 주먹 주변 ↔ HMD, 피해는 언리얼 기본 데미지(`ApplyDamage` → `OnTakeAnyDamage`, 총알 경로 `IHittable`은 그대로). 적의 몸은 C++ `AEnemyCharacter`(`ACharacter`). 탄 보급은 탄창 교체. (`Docs/PLAN.md` 6.9)
- 2026-11-03: 적 추적 목표는 플레이어 머리(HMD) 아래 바닥. C++ State Tree 전역 태스크 `FEnemyTargetTask`가 목표 위치·거리·`AttackRange`·`bInRange`를 출력하고 엔진 기본 `Move To`가 바인딩한다. State Tree는 C++ `AEnemyAIController`(`StateTreeAI` 컴포넌트, 에셋은 BP 자식)가 돌리고, 멈출 거리는 적 데이터 `AttackRange`. (`Docs/PLAN.md` 6.9 추적)
- 2026-11-04: 적 공격은 컴포넌트 `UEnemyAttack`(머리 쪽으로 돈 뒤 몽타주, 휘두르는 동안은 고정, 판정·`ApplyDamage`, `OnStrike`/`OnAttackFinished`)이 맡고, State Tree는 조건 `Enemy Can Attack` + 태스크 `Enemy Attack`으로 시작과 대기만 한다. 타격 순간·본·반경은 Anim Notify `Enemy Strike`, 몽타주 목록·피해·쿨다운은 적 데이터. (`Docs/PLAN.md` 6.9 공격)
- 2026-11-04: 공격 몽타주의 루트 모션은 살리고, 플레이어 몸 캡슐 `UPlayerBody`(HMD를 따라가는 `Pawn` 캡슐, `BP_XRPawn`에 부착)가 적을 막는다. 적 타격은 몸 캡슐 전체에 판정한다(노티파이 `Radius` 기본 15cm, 캡슐 표면까지 거리). 템플릿 폰에 충돌체가 없어 적이 플레이어를 지나쳐 타격이 머리 뒤에서 났기 때문이다. (`Docs/PLAN.md` 6.9 공격)
- 2026-11-04: 절뚝임 `Leg IK`는 절뚝일 때만 켜고 몽타주 재생 중에는 끈다(`UEnemyAnimInstance::LegIKAlpha` = `LimpAlpha` × (1 − `DefaultSlot` 몽타주 가중치)). 항상 켜 두면 발차기 발이 `ik_foot`에 묶여 바닥에 남았다. (`Docs/PLAN.md` 6.9 공격)
- 2026-11-04: 레이저 사이트는 발사 단계가 아닌 조준 보조 부품 `ULaserSightComponent`(추상)다. 잡았을 때만 켜고(BP 자식 `BP_LaserSight`가 잡기 컴포넌트 구독), `Muzzle`에 붙인 나이아가라 빔(`DynamicBeam` 템플릿에서 시작)에 매 틱 트레이스한 시작·끝 위치를 User Parameter로 넘긴다. 트레이스는 `Pawn`과 무기 자신을 무시한다. (`Docs/PLAN.md` 4.12)
- 2026-11-04: 플레이어 체력은 폰 컴포넌트 `UPlayerHealth`(`OnTakeAnyDamage` 구독, `OnHealthChanged`/`OnDied` 발행만)가 갖는다. 피격 피드백(화면 붉게·양손 진동·피격음)은 `UPlayerHitFeedback`, 남은 체력은 왼손 손목 3D 위젯이 따로 구독한다. 실패·웨이브 같은 구간 규칙은 C++ 게임 모드가 `SetPlayerDefaults`에서 `OnDied`를 구독해 맡고, 실패하면 눈앞 실패 화면(3D 위젯)의 '다시 시작'을 손 위젯 포인터로 눌러 레벨을 다시 연다. 6-5a~d로 나눠 진행. (`Docs/PLAN.md` 6.9 플레이어 체력)
- 2026-11-04: 피격 피드백 `UPlayerHitFeedback`은 `OnHealthChanged`(피해 > 0)를 구독해 카메라 포스트 프로세스 비네트(가장자리만 붉게, 세기 파라미터를 `FlashDuration` 동안 0으로, 쉴 때 가중치 0), 양손 같은 햅틱, 2D 피격음을 낸다. 피격음은 임시로 `Fire_Cue` 낮은 피치, 나중에 교체. (`Docs/PLAN.md` 6.9 플레이어 체력)
- 2026-11-05: 체력 표시는 C++ 위젯 부모 `UPlayerHealthWidget`(`UI/`, `BindWidget` `HealthBar` 필수·`BindWidgetOptional` `HealthText`, 소유 폰의 `UPlayerHealth` 구독, 연출용 `OnHealthDisplayUpdated`)이 로직, WBP가 배치. 손목 위젯은 `BP_XRPawn` 왼손 컨트롤러 아래 Widget Component로 항상 보임. `Build.cs`에 `UMG`(공개)·`Slate`/`SlateCore`(비공개). (`Docs/PLAN.md` 6.9 플레이어 체력)
- 2026-11-05: 실패 화면은 템플릿 `BP_Menu`를 복제한 `BP_FailScreen`(배치·레이저·포인터 입력은 템플릿 그대로) 안에 C++ 부모 `UFailScreenWidget`(`BindWidget` `RestartButton` → `RestartEncounter`)의 WBP를 넣는다. 게임 모드는 실패 시 `FailScreenClass`를 머리 앞에 스폰하고, `AEnemyAIController::HaltLogic`으로 레벨의 적 판단을 멈춘다(게임 모드가 적을 알고 적은 게임 모드를 모름). (`Docs/PLAN.md` 6.9 플레이어 체력)
- 2026-11-05: 웨이브는 레벨의 스폰 지점 `AEnemySpawnPoint`를 데이터 `UWaveDataAsset`이 이름(`SpawnPointName`)으로 고른다. 진행은 게임 모드 컴포넌트 `UWaveSpawner`(스폰 타이머·생존 수·다음 웨이브, 이벤트 발행만), 게임 모드는 시작 시 `StartWaves`, 모두 막으면 `Victory`, 실패 시 `StopWaves`. 첫 웨이브는 `StartDelay` 뒤 자동. 승리 화면은 따로(`VictoryScreenClass`), 부모 위젯 공유. 6-6a/b로 나눔. (`Docs/PLAN.md` 6.9 웨이브)
- 2026-11-04: 적 타격 판정은 Anim Notify State `Enemy Strike Window`(`StrikeBone`, `Radius`)의 구간 동안 본이 지나간 선분 ↔ 몸 캡슐로 하고, 한 구간에서 한 번만 맞힌다(`UEnemyAttack::Begin/Update/EndStrikeWindow`). 한 프레임 판정은 휘두르는 방향과 지나치는 타이밍 때문에 빗나갔다. (`Docs/PLAN.md` 6.9 공격)
