#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

class UAmmoDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);

UENUM(BlueprintType)
enum class EStepResult : uint8
{
	Success,
	Failed,
	Pending
};

USTRUCT(BlueprintType)
struct FIRSTVR_API FFireContext
{
	GENERATED_BODY()

	/** 이번에 발사할 탄. 탄창 단계가 기록하고, 코어가 투사체에 넘긴다. */
	UPROPERTY(BlueprintReadWrite, Category = "Fire")
	TObjectPtr<UAmmoDataAsset> Ammo;

	UPROPERTY(BlueprintReadWrite, Category = "Fire")
	float RecoilScale = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Fire")
	float DamageScale = 1.f;
};
