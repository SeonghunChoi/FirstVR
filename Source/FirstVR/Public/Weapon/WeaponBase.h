#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "WeaponBase.generated.h"

class UBoxComponent;
class USceneComponent;
class UWeaponDataAsset;
class UWeaponStepComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponFired, float, Recoil);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFireAborted);

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

	/** 지금 RequestFire를 부르면 흐름이 시작되는지(진행 중이 아니고 FireRate 간격이 지났는지). 연발 입력이 매 프레임 확인한다. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanRequestFire() const;

	/** 투사체를 발사한 직후. 반동 세기(Recoil × RecoilScale)를 넘긴다. 발사 피드백은 이 이벤트를 구독해서 준다. */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponFired OnFired;

	/** 어떤 단계가 Failed를 보고해 발사 흐름이 중단된 직후. */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponFireAborted OnFireAborted;

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

	/** 마지막 발사 뒤 FireRate 간격이 아직 지나지 않았는지. */
	bool IsCoolingDown() const;

	UFUNCTION()
	void HandleStepResult(EStepResult Result);

	/**
	 * 루트이자 무기 전체의 충돌체. 메시, 잡기 컴포넌트, Muzzle은 모두 이 아래로 들어간다.
	 * 템플릿 잡기는 잡기 컴포넌트의 부모만 손에 붙이므로, 잡기 컴포넌트는 Body 바로 아래에 둔다.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UBoxComponent> Body;

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
