#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AmmoDataAsset.generated.h"

/** 탄 종류 하나의 설정. 어떤 무기에 넣든 탄을 따라가는 값만 둔다. */
UCLASS(BlueprintType)
class FIRSTVR_API UAmmoDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 탄속(cm/s). 언리얼 단위는 1 = 1cm. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	float Speed = 35000.f;

	/** 관통력. 피격 쪽에서 적의 방어력과 비교한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0"))
	float Penetration = 1.f;

	/** 기본 피해. 피격 쪽에서 부위 배율과 DamageScale을 곱한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0"))
	float BaseDamage = 20.f;
};
