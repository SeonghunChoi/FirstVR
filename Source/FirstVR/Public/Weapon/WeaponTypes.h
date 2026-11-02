#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

class AActor;
class UAmmoDataAsset;
class USceneComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);

/** Owner의 Scene Component 중 이름이 Name인 것을 찾는다. 없으면 nullptr. */
FIRSTVR_API USceneComponent* FindSceneComponentByName(const AActor* Owner, FName Name);

UENUM(BlueprintType)
enum class EStepResult : uint8
{
	Success,
	Failed,
	Pending
};

/** 트리거 발사 모드. 단발은 당길 때마다 한 발, 연발은 당기고 있는 동안 FireRate 간격으로 계속 쏜다. */
UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single,
	Auto
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
