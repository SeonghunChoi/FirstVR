#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnPoint.generated.h"

class UArrowComponent;

/**
 * 웨이브의 적이 나오는 자리. 레벨에 배치하고 이름(SpawnPointName)을 붙이면 웨이브 데이터가 그 이름으로 고른다.
 * 액터 위치는 바닥, 화살표(X축) 방향이 적이 처음 바라보는 쪽이다. 내비 메시 안에 둔다.
 */
UCLASS()
class FIRSTVR_API AEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawnPoint();

	UFUNCTION(BlueprintPure, Category = "Encounter")
	FName GetSpawnPointName() const { return SpawnPointName; }

protected:
	/** 웨이브 데이터의 FWaveSpawnGroup::SpawnPointName과 맞춘다. 레벨 안에서 겹치지 않게 짓는다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Encounter")
	FName SpawnPointName;

	UPROPERTY(VisibleAnywhere, Category = "Encounter")
	TObjectPtr<USceneComponent> Root;

#if WITH_EDITORONLY_DATA
	/** 에디터에서만 보이는 방향 표시. */
	UPROPERTY(VisibleAnywhere, Category = "Encounter")
	TObjectPtr<UArrowComponent> Arrow;
#endif
};
