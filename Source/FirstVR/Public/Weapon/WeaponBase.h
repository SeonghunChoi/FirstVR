#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "WeaponBase.generated.h"

class USceneComponent;
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

	/** 모든 단계가 Success일 때 호출된다. Context.Ammo의 투사체를 발사 원점에서 스폰한다. */
	virtual void Fire();

	/** 무기 종류 공통 설정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> WeaponData;

private:
	void BuildSteps();
	void RunStep(int32 Index);

	UFUNCTION()
	void HandleStepResult(EStepResult Result);

	/** 루트. 블루프린트에서 붙이는 메시와 컴포넌트는 이 아래로 들어간다. */
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<USceneComponent> Root;

	/** 발사 원점. 앞 방향(X)으로 투사체가 나간다. 블루프린트에서 위치만 옮긴다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Muzzle;

	/** 마지막으로 발사한 시각(초). FireRate 제한에 쓴다. */
	double LastFireTime = -1.0e9;

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
