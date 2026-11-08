#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AmmoDataAsset.generated.h"

class AWeaponProjectile;
class UFXSystemAsset;
class USoundBase;

/** 탄 종류 하나의 설정. 어떤 무기에 넣든 탄을 따라가는 값만 둔다. */
UCLASS(BlueprintType)
class FIRSTVR_API UAmmoDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 발사할 때 스폰하는 투사체. 탄을 바꾸면 투사체 모양도 바뀐다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	TSubclassOf<AWeaponProjectile> ProjectileClass;

	/** 탄속(cm/s). 언리얼 단위는 1 = 1cm. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	float Speed = 35000.f;

	/** 관통력. 피격 쪽에서 적의 방어력과 비교한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0"))
	float Penetration = 1.f;

	/** 기본 피해. 피격 쪽에서 부위 배율과 DamageScale을 곱한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0"))
	float BaseDamage = 20.f;

	/** 맞은 바디를 미는 세기(속도 변화, cm/s). 피격 쪽에서 최종 피해 / BaseDamage 비율을 곱한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	float ImpactImpulse = 500.f;

	/**
	 * 벽·바닥처럼 피해를 받지 않는 곳에 맞았을 때의 연출(불티 등). 나이아가라 또는 캐스케이드.
	 * 맞은 면의 법선이 이펙트의 위(+Z)가 되게 놓는다. 적에 맞았을 때의 연출은 적 데이터가 정한다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Impact")
	TObjectPtr<UFXSystemAsset> ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Impact")
	FVector ImpactEffectScale = FVector::OneVector;

	/** 벽·바닥에 맞은 소리. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Impact")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Impact", meta = (ClampMin = "0.0"))
	float ImpactSoundVolume = 0.6f;
};
