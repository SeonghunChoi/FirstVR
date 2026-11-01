#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInputComponent.h"
#include "TriggerInputComponent.generated.h"

class UHapticFeedbackEffect_Base;

/**
 * 트리거 입력. 트리거가 당겨지면 코어의 RequestFire를 부른다.
 * 코어의 OnFired를 구독해 잡은 손에 발사 햅틱을 준다. 잡은 손의 입력 켜고 끄기는 부모 UWeaponInputComponent가 한다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API UTriggerInputComponent : public UWeaponInputComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void HandleAction() override;

	/** 발사할 때 잡은 손에 재생할 햅틱. 세기는 반동 세기(0~1로 제한)다. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trigger")
	TObjectPtr<UHapticFeedbackEffect_Base> FireHaptic;

private:
	UFUNCTION()
	void HandleFired(float Recoil);
};
