#pragma once

#include "CoreMinimal.h"
#include "Weapon/Steps/WeaponStepComponent.h"
#include "MagazineStep.generated.h"

class UAmmoDataAsset;

/**
 * 탄창 단계. 탄을 하나 소모하고 그 종류를 Context.Ammo에 기록한다.
 * 탄창에는 한 종류의 탄만 들어간다. 탄이 없으면 Failed.
 */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UMagazineStep : public UWeaponStepComponent
{
	GENERATED_BODY()

public:
	virtual void Execute(FFireContext& Context) override;

	/**
	 * 탄을 넣는다. 허용 탄 종류와 장탄수를 검사하고, 실제로 넣은 수를 돌려준다.
	 * 남은 탄과 다른 종류는 탄창이 비었을 때만 넣을 수 있다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
	int32 AddAmmo(UAmmoDataAsset* Ammo, int32 Count);

protected:
	virtual void BeginPlay() override;

	/** 장탄수. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine", meta = (ClampMin = "1"))
	int32 Capacity = 15;

	/** 넣을 수 있는 탄 종류. 첫 번째 항목이 배치 시 기본 탄이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
	TArray<TObjectPtr<UAmmoDataAsset>> AllowedAmmo;

	/** 배치별 초기 탄 수. BeginPlay에서 AllowedAmmo[0]으로 채운다. */
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Magazine", meta = (ClampMin = "0"))
	int32 InitialAmmo = 0;

private:
	/** 현재 탄 수. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Magazine|State", meta = (AllowPrivateAccess = "true"))
	int32 CurrentAmmo = 0;

	/** 현재 들어 있는 탄 종류. 비어 있으면 None. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Magazine|State", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAmmoDataAsset> LoadedAmmo;
};
