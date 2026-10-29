#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

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

	UPROPERTY(BlueprintReadWrite, Category = "Fire")
	float RecoilScale = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Fire")
	float DamageScale = 1.f;
};
