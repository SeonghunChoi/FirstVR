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
 * 실패하면 적의 판단을 멈추고 실패 화면을, 모두 막으면 승리 화면을 띄운다. 결과 화면의 '다시 시작'이 RestartEncounter를 부른다.
 * 웨이브 진행은 컴포넌트 UWaveSpawner가 하고, 게임 모드는 시작할 때 웨이브를 시작하고 모두 막으면 승리로 바꾼다.
 * 어떤 폰·결과 화면·웨이브 데이터를 쓸지는 블루프린트 자식에서 정한다.
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

	/**
	 * 순간이동으로 겨눈 자리(템플릿이 내비 메시에 맞춘 점)에 갈 수 있는지와 실제로 내릴 자리. 폰(BP_XRPawn)의 IsValidTeleportLocation이 부른다.
	 * 기본은 어디든 허용하고 겨눈 점 그대로다. 구간 게임 모드는 켜진 구간의 설 자리만 허용하고 그 가운데로 바꾼다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Encounter|Teleport")
	bool CanTeleportTo(FVector AimedLocation, FVector& Destination);

	/**
	 * 플레이어를 겨눈 자리로 순간이동한다. 머리(카메라)의 수평 위치가 내릴 자리에 오도록 폰을 옮기고,
	 * 그 자리에 정해진 방향이 있으면 머리가 그쪽을 보게 돌린다(없으면 보던 방향 그대로). 갈 수 없으면 false.
	 * 폰의 TryTeleport가 템플릿의 Teleport 대신 부른다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Encounter|Teleport")
	bool TeleportPlayerTo(APawn* PlayerPawn, FVector AimedLocation);

	/** 구간 상태가 바뀔 때마다. */
	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnEncounterStateChanged OnEncounterStateChanged;

protected:
	virtual void BeginPlay() override;

	/** BeginPlay에서 부른다. 기본은 웨이브 하나로 된 전투(StartWaves). 구간 게임 모드는 구간 진행으로 바꾼다. */
	virtual void StartEncounter();

	/** 웨이브 스포너가 마지막 웨이브까지 끝냈을 때. 기본은 승리. */
	UFUNCTION()
	virtual void HandleAllWavesCleared();

	/** 승리 상태로 바꾸고 승리 화면을 띄운다(진행 중일 때만). */
	void FinishWithVictory();

	/**
	 * 겨눈 자리를 실제로 내릴 자리로 바꾼다. 갈 수 없으면 false. OutFacingYaw를 채우면 도착한 머리가 그 방향을 본다.
	 * 기본은 겨눈 점 그대로, 방향 없음.
	 */
	virtual bool ResolveTeleportDestination(const FVector& AimedLocation, FVector& OutDestination, TOptional<float>& OutFacingYaw) const;

	/** 폰을 옮겨 카메라의 수평 위치를 Location에, 방향을 FacingYaw에 맞춘다. 높이는 Location의 높이(바닥). */
	static void MoveHeadTo(APawn* PlayerPawn, const FVector& Location, float FacingYaw);

	/** 폰 기준 카메라(머리)의 월드 Yaw. */
	static float GetHeadYaw(const APawn* PlayerPawn);

	/** 웨이브 진행. 웨이브 데이터는 이 컴포넌트에 넣는다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Encounter")
	TObjectPtr<UWaveSpawner> WaveSpawner;

	/** 실패하면 플레이어 눈앞에 스폰할 실패 화면 액터. 비워 두면 띄우지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter")
	TSubclassOf<AActor> FailScreenClass;

	/** 마지막 웨이브까지 막으면 스폰할 승리 화면 액터. 비워 두면 띄우지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter")
	TSubclassOf<AActor> VictoryScreenClass;

	/** 결과 화면을 머리에서 얼마나 앞(cm)에 스폰할지. 화면 액터가 스스로 자리를 다시 잡아도 된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter", meta = (ClampMin = "0.0"))
	float ResultScreenDistance = 100.f;

	/**
	 * 시작할 때 플레이어 머리(카메라)의 수평 위치·바라보는 방향을 시작 자리(PlayerStart)에 맞출지.
	 * VR 폰의 원점은 트래킹 공간의 바닥 원점이라, 플레이어가 방 안에서 걸어간 만큼 머리가 원점에서 떨어져 있다.
	 * 레벨을 다시 열어도 그 거리는 남으므로 폰을 그만큼 반대로 옮긴다(시뮬레이터 WASD 이동은 이 거리를 크게 만든다).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter")
	bool bAlignHeadToPlayerStart = true;

	/** 폰을 놓은 뒤 머리를 맞추기까지 기다릴 시간(초). 카메라가 헤드셋 자세를 받은 다음이어야 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter", meta = (ClampMin = "0.0", ForceUnits = "s", EditCondition = "bAlignHeadToPlayerStart"))
	float HeadAlignDelay = 0.2f;

private:
	/** 폰을 옮겨 카메라의 수평 위치·방향을 Start에 맞춘다. 높이는 폰의 바닥 높이 그대로. */
	void AlignHeadToStart(TWeakObjectPtr<APawn> PlayerPawn, FTransform Start);

	UFUNCTION()
	void HandlePlayerDied();

	void SetEncounterState(EEncounterState NewState);

	/** 레벨의 모든 적 판단을 멈춘다. */
	void HaltEnemies(const FString& Reason) const;

	/** 결과 화면을 머리 앞에 한 번만 스폰한다. */
	void SpawnResultScreen(TSubclassOf<AActor> ScreenClass);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Encounter|State", meta = (AllowPrivateAccess = "true"))
	EEncounterState EncounterState = EEncounterState::Playing;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Encounter|State")
	TObjectPtr<AActor> ResultScreen;
};
