#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FirstVRGameMode.generated.h"

class UWaveSpawner;

/** 전투 구간의 진행 상태. */
UENUM(BlueprintType)
enum class EEncounterState : uint8
{
	Playing,
	Failed,
	/** 마지막 웨이브까지 모두 막았다. */
	Victory
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEncounterStateChanged, EEncounterState, NewState);

/**
 * 전투 구간의 규칙을 맡는 게임 모드. 유니티의 GameManager 자리다.
 * 플레이어 폰을 준비할 때 체력 컴포넌트의 OnDied를 구독해 실패 상태로 바꾼다. 플레이어 쪽은 게임 모드를 모른다.
 * 실패하면 적의 판단을 멈추고 실패 화면을 띄운다. 실패 화면의 '다시 시작'이 RestartEncounter를 부른다.
 * 웨이브 진행은 컴포넌트 UWaveSpawner가 하고, 게임 모드는 시작할 때 웨이브를 시작하고 모두 막으면 승리로 바꾼다.
 * 어떤 폰·실패 화면·웨이브 데이터를 쓸지는 블루프린트 자식에서 정한다.
 */
UCLASS()
class FIRSTVR_API AFirstVRGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFirstVRGameMode();

	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	UFUNCTION(BlueprintPure, Category = "Encounter")
	UWaveSpawner* GetWaveSpawner() const { return WaveSpawner; }

	/** 지금 레벨을 다시 연다. 모든 상태가 처음으로 돌아간다. */
	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void RestartEncounter();

	UFUNCTION(BlueprintPure, Category = "Encounter")
	EEncounterState GetEncounterState() const { return EncounterState; }

	/** 구간 상태가 바뀔 때마다. */
	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnEncounterStateChanged OnEncounterStateChanged;

protected:
	virtual void BeginPlay() override;

	/** 웨이브 진행. 웨이브 데이터는 이 컴포넌트에 넣는다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Encounter")
	TObjectPtr<UWaveSpawner> WaveSpawner;

	/** 실패하면 플레이어 눈앞에 스폰할 실패 화면 액터. 비워 두면 띄우지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter")
	TSubclassOf<AActor> FailScreenClass;

	/** 실패 화면을 머리에서 얼마나 앞(cm)에 스폰할지. 화면 액터가 스스로 자리를 다시 잡아도 된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter", meta = (ClampMin = "0.0"))
	float FailScreenDistance = 100.f;

private:
	UFUNCTION()
	void HandlePlayerDied();

	UFUNCTION()
	void HandleAllWavesCleared();

	void SetEncounterState(EEncounterState NewState);

	/** 레벨의 모든 적 판단을 멈춘다. */
	void HaltEnemies(const FString& Reason) const;

	void SpawnFailScreen();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Encounter|State", meta = (AllowPrivateAccess = "true"))
	EEncounterState EncounterState = EEncounterState::Playing;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Encounter|State")
	TObjectPtr<AActor> FailScreen;
};
