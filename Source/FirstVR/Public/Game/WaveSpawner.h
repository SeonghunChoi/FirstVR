#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaveSpawner.generated.h"

class AEnemyCharacter;
class AEnemySpawnPoint;
class UWaveDataAsset;
struct FWaveSpawnGroup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStarted, int32, WaveNumber, int32, WaveCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCleared, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCleared);

/**
 * 웨이브 진행. 게임 모드가 갖는 컴포넌트다. WaveData 순서대로 스폰 지점에서 적을 내보내고,
 * 웨이브의 적이 모두 나오고 모두 쓰러지면 쉬었다가 다음 웨이브, 마지막이면 OnAllWavesCleared를 발행한다.
 * 승리·실패 판단은 게임 모드가 이 이벤트를 구독해서 한다.
 */
UCLASS(ClassGroup = (Encounter), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UWaveSpawner : public UActorComponent
{
	GENERATED_BODY()

public:
	UWaveSpawner();

	/** WaveData의 StartDelay 뒤에 첫 웨이브를 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "Encounter|Wave")
	void StartWaves();

	/** 남은 스폰과 다음 웨이브 예약을 모두 취소한다. 이미 나온 적은 건드리지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Encounter|Wave")
	void StopWaves();

	/** 지금 웨이브 번호(1부터). 아직 시작 전이면 0. */
	UFUNCTION(BlueprintPure, Category = "Encounter|Wave")
	int32 GetCurrentWaveNumber() const { return CurrentWaveIndex + 1; }

	UFUNCTION(BlueprintPure, Category = "Encounter|Wave")
	int32 GetWaveCount() const;

	UFUNCTION(BlueprintPure, Category = "Encounter|Wave")
	int32 GetAliveEnemyCount() const { return AliveEnemies.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "Encounter|Wave")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Encounter|Wave")
	FOnWaveCleared OnWaveCleared;

	UPROPERTY(BlueprintAssignable, Category = "Encounter|Wave")
	FOnAllWavesCleared OnAllWavesCleared;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 이 구간의 웨이브 목록. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter|Wave")
	TObjectPtr<UWaveDataAsset> WaveData;

private:
	/** 묶음마다 남은 적 수와 스폰 타이머. */
	struct FGroupProgress
	{
		int32 Remaining = 0;
		FTimerHandle Timer;
	};

	void ScheduleWave(int32 WaveIndex, float Delay);
	void StartWave(int32 WaveIndex);
	void SpawnFromGroup(int32 GroupIndex);
	AEnemyCharacter* SpawnEnemy(const FWaveSpawnGroup& Group) const;
	AEnemySpawnPoint* FindSpawnPoint(FName SpawnPointName) const;

	UFUNCTION()
	void HandleEnemyDied();

	/** 모든 묶음이 다 나왔고 살아 있는 적이 없으면 웨이브를 끝낸다. */
	void CheckWaveCleared();

	TArray<FGroupProgress> GroupProgress;
	FTimerHandle WaveTimer;

	/** 웨이브가 진행 중인지(시작했고 아직 끝나지 않음). */
	bool bWaveActive = false;

	/** StartWaves 뒤 StopWaves나 마지막 웨이브 전까지. */
	bool bRunning = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Encounter|Wave|State")
	int32 CurrentWaveIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Encounter|Wave|State")
	TArray<TObjectPtr<AEnemyCharacter>> AliveEnemies;
};
