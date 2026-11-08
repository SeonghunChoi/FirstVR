#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EncounterSection.generated.h"

class UArrowComponent;
class UBoxComponent;
class UStaticMeshComponent;
class UWaveDataAsset;
class AEncounterSection;

/** 구간의 진행 단계. */
UENUM(BlueprintType)
enum class ESectionState : uint8
{
	/** 아직 차례가 아니다. 표식이 꺼져 있다. */
	Waiting,
	/** 차례가 됐다. 표식이 켜지고 플레이어가 설 자리에 오기를 기다린다. */
	Active,
	/** 플레이어가 도착해 이 구간의 웨이브를 막는 중이다. */
	Fighting,
	/** 끝났다. 문을 풀었다. */
	Cleared
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSectionPlayerArrived, AEncounterSection*, Section);

/**
 * 레벨에 놓는 전투 구간. 순서, 플레이어가 설 자리(도착 상자·표식), 이 구간의 웨이브 데이터, 끝내면 풀 문, 끝낼 때 회복량을 갖는다.
 * 진행 순서는 구간 게임 모드(AFirstVRSectionGameMode)가 정하고, 구간은 자기 상태와 도착 판정·문 잠금만 맡는다.
 * 웨이브 데이터가 비어 있으면 도착하는 순간 끝나는 구간이다(탈출 지점, 이동만 하는 자리).
 * 문은 팩 블루프린트(BP_BaseDoor)를 그대로 쓰므로 bool 변수 이름으로 찾는다. 시작할 때 잠그고(닫힌 채), 끝내면 DoorOpenPropertyName으로 연 뒤
 * 잠금은 그대로 둔다. 팩 문은 잠겨 있지 않으면 다가갈 때마다 처음부터 다시 열리는 애니메이션을 틀고 멀어지면 닫히기 때문이다.
 */
UCLASS()
class FIRSTVR_API AEncounterSection : public AActor
{
	GENERATED_BODY()

public:
	AEncounterSection();

	UFUNCTION(BlueprintPure, Category = "Encounter|Section")
	int32 GetSectionOrder() const { return SectionOrder; }

	UFUNCTION(BlueprintPure, Category = "Encounter|Section")
	UWaveDataAsset* GetWaveData() const { return WaveData; }

	/** 웨이브 없이 도착하면 끝나는 구간인지. */
	UFUNCTION(BlueprintPure, Category = "Encounter|Section")
	bool HasWaves() const;

	UFUNCTION(BlueprintPure, Category = "Encounter|Section")
	float GetHealOnClear() const { return HealOnClear; }

	UFUNCTION(BlueprintPure, Category = "Encounter|Section")
	ESectionState GetSectionState() const { return SectionState; }

	/** 월드 위치가 설 자리(ArrivalBox) 안인지, 수평으로만 본다. 도착 판정과 순간이동 허용에 쓴다. */
	UFUNCTION(BlueprintPure, Category = "Encounter|Section")
	bool IsInsideStandArea(const FVector& WorldLocation) const;

	/** 차례가 됐다. 표식을 켜고, 플레이어가 이미 설 자리에 있으면 바로 도착으로 친다. */
	UFUNCTION(BlueprintCallable, Category = "Encounter|Section")
	void Activate();

	/** 도착을 강제로 일으킨다(Encounter.ArriveNext 시험용, VR 없이 일반 Play에서). Active일 때만. */
	UFUNCTION(BlueprintCallable, Category = "Encounter|Section")
	void ForceArrive();

	/** 끝났다. 표식을 끄고 문을 연다(열린 채 잠가 둔다). */
	UFUNCTION(BlueprintCallable, Category = "Encounter|Section")
	void MarkCleared();

	/** 플레이어가 Active인 구간의 설 자리에 들어왔을 때 한 번. */
	UPROPERTY(BlueprintAssignable, Category = "Encounter|Section")
	FOnSectionPlayerArrived OnPlayerArrived;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Encounter|Section")
	TObjectPtr<USceneComponent> Root;

	/**
	 * 설 자리 범위. 플레이어 머리(HMD) 위치가 이 상자 안(수평)에 들어오면 도착이다. 충돌은 쓰지 않고 모양만 쓴다.
	 * 겹침 이벤트는 "들어오는 순간"만 알려 줘서, 처음부터 서 있거나 몸 충돌이 늦게 켜지면 놓쳤다(8-6b VR Preview).
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Encounter|Section")
	TObjectPtr<UBoxComponent> ArrivalBox;

	/** 설 자리 표식. Active일 때만 보인다. 메시는 블루프린트 자식에서 정한다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Encounter|Section")
	TObjectPtr<UStaticMeshComponent> Marker;

#if WITH_EDITORONLY_DATA
	/** 순간이동으로 도착한 플레이어가 바라볼 방향(액터 방향). 에디터에서만 보인다. */
	UPROPERTY(VisibleAnywhere, Category = "Encounter|Section")
	TObjectPtr<UArrowComponent> FacingArrow;
#endif

	/** 진행 순서. 작은 것부터. 레벨 안에서 겹치지 않게. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Encounter|Section")
	int32 SectionOrder = 0;

	/** 이 구간의 웨이브. 비우면 도착하는 순간 끝난다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Encounter|Section")
	TObjectPtr<UWaveDataAsset> WaveData;

	/** 이 구간을 끝내면 풀 문. 시작할 때 잠근다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Encounter|Section")
	TArray<TObjectPtr<AActor>> DoorsToUnlock;

	/** 이 구간을 끝내면 플레이어 체력을 얼마나 채울지. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Encounter|Section", meta = (ClampMin = "0.0"))
	float HealOnClear = 50.f;

	/** 문 블루프린트의 잠금 bool 변수 이름(팩 BP_BaseDoor는 Locked). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter|Section")
	FName DoorLockPropertyName = TEXT("Locked");

	/** 끝낼 때 문을 열게 켜는 bool 변수 이름(팩 BP_BaseDoor는 NeedOpen, 다음 틱에 열린다). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter|Section")
	FName DoorOpenPropertyName = TEXT("NeedOpen");

	/** 문이 열려 있는지 나타내는 bool 변수 이름(팩 BP_BaseDoor는 Opened). 이미 열려 있으면 열기를 다시 시키지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter|Section")
	FName DoorOpenedPropertyName = TEXT("Opened");

	/** Active인 동안 도착을 확인하는 간격(초). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter|Section", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float ArrivalCheckInterval = 0.2f;

	/** 상태가 바뀔 때(표식 연출 등은 블루프린트에서). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Encounter|Section")
	void OnSectionStateChanged(ESectionState NewState);

private:
	/** 플레이어 머리가 설 자리 안이면 도착시킨다. */
	void CheckArrival();
	void Arrive();
	void LockDoorsClosed() const;
	void OpenDoors() const;
	static bool SetDoorBool(AActor* Door, FName PropertyName, bool bValue);
	static bool GetDoorBool(const AActor* Door, FName PropertyName, bool& bOutValue);
	void SetSectionState(ESectionState NewState);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Encounter|Section|State", meta = (AllowPrivateAccess = "true"))
	ESectionState SectionState = ESectionState::Waiting;

	FTimerHandle ArrivalTimer;
};
