#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

class UWeaponStepComponent;

UCLASS(BlueprintType)
class FIRSTVR_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 발사 요청마다 이 순서대로 단계를 실행한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	TArray<TSubclassOf<UWeaponStepComponent>> FireSequence;

	/** 기본 반동 세기. FFireContext::RecoilScale을 곱해 쓴다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "0.0"))
	float Recoil = 1.f;

	/** 초당 발사 횟수. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "0.1"))
	float FireRate = 5.f;
};
