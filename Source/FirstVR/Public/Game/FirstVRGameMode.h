#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FirstVRGameMode.generated.h"

/** 전투 구간의 진행 상태. 웨이브와 승리는 6-6에서 더한다. */
UENUM(BlueprintType)
enum class EEncounterState : uint8
{
	Playing,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEncounterStateChanged, EEncounterState, NewState);

/**
 * 전투 구간의 규칙을 맡는 게임 모드. 유니티의 GameManager 자리다.
 * 플레이어 폰을 준비할 때 체력 컴포넌트의 OnDied를 구독해 실패 상태로 바꾼다. 플레이어 쪽은 게임 모드를 모른다.
 * 어떤 폰·실패 화면을 쓸지는 블루프린트 자식에서 정한다.
 */
UCLASS()
class FIRSTVR_API AFirstVRGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	/** 지금 레벨을 다시 연다. 모든 상태가 처음으로 돌아간다. */
	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void RestartEncounter();

	UFUNCTION(BlueprintPure, Category = "Encounter")
	EEncounterState GetEncounterState() const { return EncounterState; }

	/** 구간 상태가 바뀔 때마다. */
	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnEncounterStateChanged OnEncounterStateChanged;

private:
	UFUNCTION()
	void HandlePlayerDied();

	void SetEncounterState(EEncounterState NewState);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Encounter|State", meta = (AllowPrivateAccess = "true"))
	EEncounterState EncounterState = EEncounterState::Playing;
};
