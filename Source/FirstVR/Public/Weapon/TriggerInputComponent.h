#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInputComponent.h"
#include "Weapon/WeaponTypes.h"
#include "TriggerInputComponent.generated.h"

class UHapticFeedbackEffect_Base;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireModeChanged, EFireMode, NewMode);

/**
 * 트리거 입력. 트리거가 당겨지면 코어의 RequestFire를 부른다.
 * 발사 모드가 연발이면 당기고 있는 동안 코어가 받을 수 있을 때마다(CanRequestFire) 다시 요청하고, 발사가 중단되면(OnFireAborted) 다시 당길 때까지 멈춘다.
 * 코어의 OnFired를 구독해 잡은 손에 발사 햅틱을 준다. 잡은 손의 입력 켜고 끄기는 부모 UWeaponInputComponent가 한다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API UTriggerInputComponent : public UWeaponInputComponent
{
	GENERATED_BODY()

public:
	UTriggerInputComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 발사 모드를 바꾼다. FireModes에 없는 모드면 바꾸지 않고 false. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Trigger")
	bool SetFireMode(EFireMode NewMode);

	/** 지금 발사 모드. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Trigger")
	EFireMode GetFireMode() const { return FireMode; }

	/** 이 무기가 쓸 수 있는 발사 모드(조정간 순서). */
	UFUNCTION(BlueprintPure, Category = "Weapon|Trigger")
	const TArray<EFireMode>& GetFireModes() const { return FireModes; }

	/** 발사 모드가 바뀌었을 때. 같은 모드로 설정하면 발행하지 않는다. */
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Trigger")
	FOnFireModeChanged OnFireModeChanged;

protected:
	virtual void BeginPlay() override;
	virtual void HandleAction() override;
	virtual void HandleActionReleased() override;

	/** 발사할 때 잡은 손에 재생할 햅틱. 세기는 반동 세기(0~1로 제한)다. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trigger")
	TObjectPtr<UHapticFeedbackEffect_Base> FireHaptic;

	/** 이 무기가 쓸 수 있는 발사 모드. 조정간이 이 순서로 돌린다. 첫 항목이 시작 모드다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Trigger")
	TArray<EFireMode> FireModes = { EFireMode::Single };

private:
	/** 연발 반복을 멈춘다. 다시 당길 때까지 요청하지 않는다. */
	void StopAutoFire(const TCHAR* Reason);

	UFUNCTION()
	void HandleFired(float Recoil);

	UFUNCTION()
	void HandleFireAborted();

	/** 지금 발사 모드. */
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Trigger|State")
	EFireMode FireMode = EFireMode::Single;

	/** 연발로 반복 요청 중인지. 반복 중에만 틱이 켜진다. */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Weapon|Trigger|State")
	bool bAutoFiring = false;
};
