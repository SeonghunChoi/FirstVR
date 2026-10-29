#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "WeaponBase.generated.h"

class UWeaponDataAsset;
class UWeaponStepComponent;

UCLASS(Abstract)
class FIRSTVR_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	/** 발사 흐름을 처음 단계부터 시작한다. 이미 진행 중이면 무시한다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RequestFire();

	/** 발사 흐름이 진행 중(대기 포함)인지. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsFiring() const { return CurrentStepIndex != INDEX_NONE; }

protected:
	virtual void BeginPlay() override;

	/** 모든 단계가 Success일 때 호출된다. Step 2-3에서 투사체 발사로 채운다. */
	virtual void Fire();

	/** 무기 종류 공통 설정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> WeaponData;

private:
	void BuildSteps();
	void RunStep(int32 Index);

	UFUNCTION()
	void HandleStepResult(EStepResult Result);

	/** FireSequence 순서대로 찾은 단계 컴포넌트. */
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|State")
	TArray<TObjectPtr<UWeaponStepComponent>> Steps;

	/** 실행 중인 단계 번호. 진행 중이 아니면 INDEX_NONE(-1). */
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|State")
	int32 CurrentStepIndex = INDEX_NONE;

	/** 이번 발사에서 단계들이 기록하는 보고서. */
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|State")
	FFireContext Context;
};
