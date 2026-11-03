# 무기 조립 시나리오

> 컴포넌트 조합형 무기 시스템이 실제로 "조립"으로 새 무기를 만든다는 것을 보여 주는 자료.
> `Docs/PLAN.md` 9.2의 PDF 3페이지(무기별 컴포넌트 조합표와 발사 흐름)의 재료다.
> 스크린샷은 `Docs/Images/Assembly/`에 둔다.

---

## 1. 조립 규칙

무기 하나는 아래 네 가지를 블루프린트와 데이터 에셋에서 고르는 것으로 정해진다.

| 구성 | 무엇을 정하나 | 어디서 |
|---|---|---|
| **코어** `AWeaponBase` | 발사 요청을 받아 단계를 순서대로 실행하고, 끝까지 Success면 탄을 쏜다 | 모든 무기의 부모 (C++) |
| **발사 단계** `UWeaponStepComponent` 자식들 | 이번 한 발을 쏴도 되는지 (Success / Failed / Pending) | 블루프린트에 컴포넌트로 부착 |
| **순서** `UWeaponDataAsset::FireSequence` | 어떤 단계를 어떤 순서로 거치는지 | 무기 데이터 에셋 |
| **입력** `UWeaponInputComponent` 자식들 | 잡은 손의 버튼을 어떤 동작에 연결하는지 | 블루프린트에 컴포넌트로 부착 |
| **기준점** | `Muzzle`(코어가 만듦), `Grip`, `SupportGrip`, `MagazineWell` | 이름으로 찾는 Scene Component |

- 코어는 단계의 구체 타입을 모른다. `FireSequence`에 적힌 클래스로 자기 컴포넌트를 찾아(`FindComponentByClass`) 순서대로 `Execute`를 부를 뿐이다.
- 데이터에는 부모 클래스를 적어도 된다. `FireSequence`의 `MagazineStep`(C++)은 블루프린트에 붙은 `BP_MagazineStep`(빈 격발 피드백이 있는 자식)을 찾는다.
- 데이터가 요구하는 단계가 블루프린트에 없으면 코어가 `No component for step ... Weapon cannot fire.`를 남기고 그 무기는 쏘지 않는다.

---

## 2. 조합표

행은 무기, 열은 컴포넌트다. ●는 부착, 숫자는 `FireSequence` 순서다.

| | 권총 | 단발 소총 | 연발 소총 |
|---|:-:|:-:|:-:|
| 블루프린트 | `BP_PistolWeapon` | `BP_RifleWeapon` | `BP_AutoRifleWeapon` |
| 무기 데이터 | `DA_Pistol` | `DA_Rifle` | `DA_Rifle` |
| 탄 데이터 | `DA_Ammo_9mm` | `DA_Ammo_556` | `DA_Ammo_556` |
| `Recoil` / `FireRate`(발/초) | 1.0 / 3 | 1.5 / 6 | 1.5 / 6 |
| 외형 (기본 도형) | `Cube`, `Grab` | `Cube`, `Grab`, `Shoulder` | `Receiver`, `Barrel`, `Grab`, `Gunstock` |
| **기준점** | | | |
| `Body`·`Muzzle` (코어) | ● | ● | ● |
| `Grip` (Snap 잡기) | ● | ● | ● |
| `SupportGrip` (Custom 잡기) | | ● | ● |
| `MagazineWell` (자리만) | ● | ● | ● |
| **발사 단계** | | | |
| `BP_GripStep` 파지 | 1 | 1 | 1 |
| `BP_TwoHandGripStep` 두 손 | | 2 | 2 |
| `BP_SafetyStep` 안전장치 | | 3 | 3 |
| `BP_MagazineStep` 탄창 | 2 | 4 | 4 |
| **입력** | | | |
| `BP_TriggerInput` 트리거 (`FireModes`) | ● `[Single]` | ● `[Single]` | ● `[Single, Auto]` |
| `BP_SafetyInput` 안전장치 토글 (A/X) | | ● | |
| `BP_SelectorInput` 조정간 (A/X) | | | ● |

### 발사 흐름

```
권총      = 파지 → 탄창 → 발사
단발 소총 = 파지 → 두 손 → 안전장치 → 탄창 → 발사
연발 소총 = 파지 → 두 손 → 안전장치 → 탄창 → 발사   (트리거: 단발/연발, 조정간: 안전 → 단발 → 연발)
```

- 단발 소총과 연발 소총은 **발사 단계와 데이터가 같다.** 다른 것은 입력 컴포넌트 하나(안전장치 토글 → 조정간)와 트리거의 `FireModes` 설정값뿐이다.
- 외형은 기본 도형 메시를 `Body` 아래에 쌓은 것이고 시스템은 메시를 참조하지 않는다. 발사·파지·결합 위치는 이름이 정해진 기준점(`Muzzle`, `Grip`, `SupportGrip`, `MagazineWell`)으로 찾으므로, 나중에 Fab 메시로 바꿔도 기준점 위치만 옮기면 된다.
- 두 손 파지는 발사 조건이 아니다. 항상 Success이고 반동 배율(`RecoilScale`, 한 손 1.0 / 두 손 0.4)만 기록한다.

### 스크린샷

| 권총 | 단발 소총 | 연발 소총 |
|---|---|---|
| ![](Images/Assembly/pistol_components.png) | ![](Images/Assembly/rifle_components.png) | ![](Images/Assembly/autorifle_components.png) |

| `DA_Pistol` | `DA_Rifle` | 연발 소총 `BP_TriggerInput` |
|---|---|---|
| ![](Images/Assembly/da_pistol.png) | ![](Images/Assembly/da_rifle.png) | ![](Images/Assembly/autorifle_firemodes.png) |

---

## 3. 무기를 만들 때 늘어난 C++

"새 무기에 C++이 0줄"이라는 말은 **그 무기를 조립하는 시점**의 이야기다. 새 **기능**이 필요할 때는 C++ 컴포넌트를 하나 추가하고, 그 뒤의 무기는 조립만 한다.

| 무기 | 조립 전에 기능으로 추가한 C++ | 조립할 때 새 C++ |
|---|---|---|
| 권총 (Step 1~3) | 코어 `AWeaponBase`, `UWeaponStepComponent`, `UWeaponDataAsset`, `UAmmoDataAsset`, `AWeaponProjectile`, `UGripStep`, `UMagazineStep`, `UTriggerInputComponent` | — (첫 무기) |
| 단발 소총 (5-4a) | `USafetyStep`(5-1), `UWeaponInputComponent`·`USafetyInputComponent`(5-2), `UTwoHandGripStep`(5-3) | **0줄** |
| 연발 소총 (5-4b) | `EFireMode`, `UTriggerInputComponent` 발사 모드, `USelectorInputComponent`, 코어 `CanRequestFire`·`OnFireAborted`(구체 타입을 모르는 일반 질의·알림) | **0줄** |
| 빈 무기에서 다시 조립 (5-5b) | 없음 | **0줄** |

- 기능을 추가하면서 **코어가 구체 단계·입력 타입을 알게 된 적은 없다.** 코어에 늘어난 것은 "지금 요청을 받을 수 있나"(`CanRequestFire`)와 "발사가 중단됐다"(`OnFireAborted`)뿐이다.
- 이미 있던 단계(파지, 탄창, 안전장치, 두 손)는 새 무기를 위해 고친 적이 없다.

---

## 4. 빈 무기에서 조립하기 (5-5b)

`AWeaponBase`를 부모로 한 빈 블루프린트에서 시작해, 한 번에 한 가지씩 붙이고 VR Preview로 확인한다. 끝나면 연발 소총과 같은 조합이 된다. 모든 단계에서 C++ 빌드는 없다.

| 단계 | 블루프린트에 붙이는 것 | 데이터 변경 | 확인 |
|---|---|---|---|
| 0. 빈 무기 | (없음: 코어의 `Body`·`Muzzle`만) | 무기 데이터 비움 | `WeaponData is not set` |
| 1. 잡기와 트리거 | 외형 `Cube`, `Grip`, `BP_GripStep`, `BP_TriggerInput` | 새 무기 데이터, `FireSequence`=[`BP_GripStep`] | 잡고 당기면 `FIRE (Ammo None ...)` + `No ammo in context` (단계는 돌지만 쏠 탄이 없다) |
| 2. 탄창 | `BP_MagazineStep` (30발, `DA_Ammo_556`), 레벨 인스턴스 `Initial Ammo` 3 | `FireSequence` 끝에 `MagazineStep` | 탄이 나가고 3 → 0, 네 번째에 `Empty` → `Click! (empty)` → `Fire aborted at step 2/2` |
| 3. 안전장치 | `BP_SafetyStep`, `BP_SafetyInput` | 탄창 앞에 `BP_SafetyStep` | 데이터만 먼저 바꾸면 `No component for step`, 붙이면 잠김에서 중단, `B`로 풀면 발사 |
| 4. 두 손 | `SupportGrip`, `BP_TwoHandGripStep` | 파지 뒤에 `BP_TwoHandGripStep` | 두 손으로 잡으면 `Recoil` 1.50 → 0.60, 두 손 조준 |
| 5. 조정간과 연발 | `BP_SafetyInput` 빼고 `BP_SelectorInput`, 트리거 `FireModes`=[`Single`, `Auto`] | 없음 | `B`로 안전 → 단발 → 연발, 연발 반복과 멈춤 |

결과물은 `Portfolio/Weapons/BP_AssembledRifle` + `Portfolio/Data/DA_AssembledRifle`이다. 확인은 모두 VR Preview, 출력 로그 필터는 `LogWeapon` + `LogBlueprintUserMessages`(단계 피드백의 Print String).

### 단계 0. 빈 무기

`AWeaponBase`를 부모로 블루프린트를 만들기만 한 상태. 코어가 만든 `Body`와 `Muzzle`만 있다.

```
Error: [BP_AssembledRifle_C_1] WeaponData is not set
```

| 컴포넌트 |
|---|
| ![](Images/Assembly/step0_components.png) |

### 단계 1. 잡기와 트리거

외형(`Receiver` 아래 `Barrel`·`Grab`·`Gunstock`), `Grip`(Snap), `BP_GripStep`, `BP_TriggerInput`을 붙이고 새 무기 데이터 `DA_AssembledRifle`(`FireSequence`=[`BP_GripStep`], `Recoil` 1.5, `FireRate` 6)을 연결한다.

```
1 steps ready
[잡고 트리거] Fire requested → [BP_GripStep] Success
             FIRE (Ammo None, Recoil 1.50, DamageScale 1.00)
             Warning: No ammo in context. Is a magazine step in FireSequence?
```

단계는 끝까지 Success지만 쏠 탄이 없어 투사체도 햅틱도 없다. "무엇을 쏘는가"는 탄창 단계가 `Context.Ammo`로 알려 준다.

| 컴포넌트 | 무기 데이터 연결 |
|---|---|
| ![](Images/Assembly/step1_components.png) | ![](Images/Assembly/step1_data.png) |

### 단계 2. 탄창

`BP_MagazineStep`(`Capacity` 30, `AllowedAmmo`=[`DA_Ammo_556`])을 붙이고 데이터 끝에 `MagazineStep`(C++ 부모)을 넣는다. 레벨 인스턴스의 `Initial Ammo`는 3.

```
2 steps ready
[트리거 ×3] [BP_MagazineStep] DA_Ammo_556, 2/30 → 1/30 → 0/30 left, 매번 FIRE + Launch + 햅틱
[트리거 4번째] [BP_MagazineStep] Empty → Click! (empty) → Fire aborted at step 2/2
```

| 컴포넌트 | 데이터 | 인스턴스 `Initial Ammo` |
|---|---|---|
| ![](Images/Assembly/step2_components.png) | ![](Images/Assembly/step2_data.png) | ![](Images/Assembly/step2_instance.png) |

### 단계 3. 안전장치

데이터에 `BP_SafetyStep`만 먼저 넣으면 코어가 부족한 단계를 알려 주고 쏘지 않는다.

```
Error: No component for step BP_SafetyStep_C. Weapon cannot fire.
```

`BP_SafetyStep`과 `BP_SafetyInput`을 붙이면:

```
3 steps ready
[트리거] Safety on → Safety is on → Fire aborted at step 2/3
[B]      Safety released
[트리거] 2/30 left → FIRE (Recoil 1.50) → haptic 1.00 (left hand)
[B]      Safety locked → [트리거] Fire aborted at step 2/3
```

| 컴포넌트 | 데이터 |
|---|---|
| ![](Images/Assembly/step3_components.png) | ![](Images/Assembly/step3_data.png) |

### 단계 4. 두 손

`SupportGrip`(템플릿 `BP_GrabComponent`, Custom)과 `BP_TwoHandGripStep`을 붙이고 데이터의 파지 뒤에 `BP_TwoHandGripStep`을 넣는다.

```
4 steps ready
[한 손] One hand, recoil scale x1.00 → Safety on → Fire aborted at step 3/4
        B → One hand x1.00 → FIRE (Recoil 1.50) → haptic 1.00
[두 손] Support held (MotionControllerRightGrip) → Two-hand aim on (parent MotionControllerLeftGrip)
        Two hands, recoil scale x0.40 → FIRE (Recoil 0.60) → haptic 0.60
[놓기]  Support released → Two-hand aim off, one-hand pose restored
```

- 단계 하나를 끼워 넣자 안전장치에서 멈추는 위치가 2/3에서 3/4로 바뀌었다. 순서는 데이터에만 있다.
- `SupportGrip`과 `BP_TwoHandGripStep`은 **둘 다** 있어야 한다. `SupportGrip`은 잡았다/놓았다만 알리는 템플릿 잡기이고, 그 신호로 반동 배율·두 손 조준·보조 손 들기를 하는 것은 `BP_TwoHandGripStep`이다. 단계 없이 `SupportGrip`만 붙이면 보조 손으로 잡아도 아무 일이 없고, 데이터에만 단계를 넣으면 `No component for step`으로 쏘지 않는다. (`Grip`과 `BP_GripStep`의 관계도 같다.)

| 컴포넌트 | 데이터 |
|---|---|
| ![](Images/Assembly/step4_components.png) | ![](Images/Assembly/step4_data.png) |

### 단계 5. 조정간과 연발

`BP_SafetyInput`을 지우고 `BP_SelectorInput`을 붙인 뒤, `BP_TriggerInput`의 `Fire Modes`를 [`Single`, `Auto`]로 늘린다. **데이터는 바꾸지 않는다.** 레벨 인스턴스의 `Initial Ammo`는 10.

```
4 steps ready
[트리거]   Safety on → Fire aborted at step 3/4
[B]        Safety released → Selector EFireMode::Single
[트리거]   9/30 left → FIRE 한 발
[B]        Fire mode EFireMode::Auto → Selector EFireMode::Auto
[누르고 있기] Auto fire started → 8 → 7 → … → 1/30 left (약 0.18초 간격 FIRE 8발)
[떼기]     Auto fire stopped (trigger released)
[B]        Safety locked → Selector Safe
[트리거]   Auto fire started → Safety on → Fire aborted at step 3/4 → Auto fire stopped (fire aborted)
[B ×2]     Safety released → Selector Single → Fire mode Auto → Selector Auto
[누르고 있기] 0/30 left → FIRE → Empty → Click! (empty) → Fire aborted at step 4/4 → Auto fire stopped (fire aborted)
```

- 연발 간격은 데이터의 `FireRate` 6(약 0.17초)을 코어 쿨다운이 지킨 결과다. 트리거는 매 틱 `CanRequestFire`만 묻는다.
- 안전에서도 트리거의 현재 모드는 연발로 남아 있어 당기면 `Auto fire started`가 찍히지만, 첫 요청이 안전장치 단계에서 Failed되어 코어의 `OnFireAborted`로 바로 멈춘다. 연발 반복이 안전장치를 우회하지 않는다.
- 5-4b에서 확인하지 못했던 연발 → 안전 순환을 여기서 확인했다.

| 컴포넌트 | 트리거 `Fire Modes` |
|---|---|
| ![](Images/Assembly/step5_components.png) | ![](Images/Assembly/step5_firemodes.png) |

### 결과: 연발 소총과 대조

| | `BP_AutoRifleWeapon` (5-4b) | `BP_AssembledRifle` (5-5b) |
|---|---|---|
| 발사 단계 | `BP_GripStep`, `BP_TwoHandGripStep`, `BP_SafetyStep`, `BP_MagazineStep` | 같음 |
| 입력 | `BP_TriggerInput` `[Single, Auto]`, `BP_SelectorInput` | 같음 |
| `FireSequence` | `DA_Rifle`: 파지 → 두 손 → 안전장치 → 탄창 | `DA_AssembledRifle`: 같은 순서 |
| `Recoil` / `FireRate` | 1.5 / 6 | 1.5 / 6 |
| 기준점 | `Grip`, `SupportGrip`, `MagazineWell`, `Muzzle` | `Grip`, `SupportGrip`, `Muzzle` (`MagazineWell`은 아직 쓰는 시스템이 없는 자리 표시라 생략) |
| 새 C++ | 0줄 | **0줄** |

빈 `AWeaponBase`에서 시작해 컴포넌트 6개와 데이터 에셋 하나로 연발 소총을 다시 만들었고, 그동안 C++ 빌드는 한 번도 하지 않았다.
