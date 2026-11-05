#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WaveDataAsset.generated.h"

class AEnemyCharacter;

/** 한 웨이브 안에서 한 스폰 지점으로 나오는 적 묶음. */
USTRUCT(BlueprintType)
struct FWaveSpawnGroup
{
	GENERATED_BODY()

	/** 내보낼 적 블루프린트. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	/** 이 묶음에서 나올 적의 수. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "1"))
	int32 Count = 1;

	/** 레벨에 놓인 AEnemySpawnPoint의 SpawnPointName. 데이터 에셋은 레벨의 액터를 직접 가리킬 수 없어서 이름으로 고른다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	FName SpawnPointName;

	/** 웨이브가 시작된 뒤 이 묶음의 첫 적이 나오기까지(초). 묶음마다 엇갈려 내보낼 때 쓴다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.0"))
	float StartDelay = 0.f;

	/** 묶음 안에서 적 사이의 간격(초). 0이면 한꺼번에 나온다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.0"))
	float SpawnInterval = 2.f;
};

/** 웨이브 하나. 모든 묶음이 다 나오고 모두 쓰러지면 끝난다. */
USTRUCT(BlueprintType)
struct FWaveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<FWaveSpawnGroup> Groups;

	/** 이 웨이브를 다 막은 뒤 다음 웨이브까지 쉬는 시간(초). 마지막 웨이브에서는 쓰지 않는다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.0"))
	float RestAfterClear = 5.f;
};

/**
 * 전투 구간의 웨이브 목록. 내용은 데이터, 진행 규칙은 UWaveSpawner(게임 모드 컴포넌트)가 맡는다.
 * 무기의 UWeaponDataAsset처럼 유니티의 ScriptableObject 자리다.
 */
UCLASS(BlueprintType)
class FIRSTVR_API UWaveDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 게임이 시작된 뒤 첫 웨이브까지(초). 무기를 집을 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.0"))
	float StartDelay = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave")
	TArray<FWaveDefinition> Waves;
};
