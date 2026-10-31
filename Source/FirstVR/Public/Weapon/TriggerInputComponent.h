#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "TriggerInputComponent.generated.h"

class UHapticFeedbackEffect_Base;
class UInputAction;
class UInputMappingContext;
class USceneComponent;

/**
 * 트리거 입력. 무기가 잡히면 잡은 손의 입력을 켜고, 트리거가 당겨지면 코어의 RequestFire를 부른다.
 * 코어의 OnFired를 구독해 잡은 손에 발사 햅틱을 준다. 발사 흐름의 단계가 아니다. 템플릿 잡기와의 연결(어느 손인지)은 블루프린트 자식(BP_TriggerInput)이 한다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API UTriggerInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTriggerInputComponent();

	/** 잡은 손의 트리거 입력을 켠다. 이미 켜져 있으면 먼저 끈다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Trigger")
	void EnableForHand(bool bRightHand);

	/** 트리거 입력을 끈다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Trigger")
	void DisableTrigger();

	/** 같은 액터에서 이름이 GripComponentName인 Scene Component를 찾는다. 없으면 None. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Trigger")
	USceneComponent* FindGripComponent() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 발사할 때 잡은 손에 재생할 햅틱. 세기는 반동 세기(0~1로 제한)다. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trigger")
	TObjectPtr<UHapticFeedbackEffect_Base> FireHaptic;

	/** 이 입력을 켜고 끌 기준이 되는 잡기 컴포넌트의 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Trigger")
	FName GripComponentName = TEXT("Grip");

	/** 왼손으로 잡았을 때 추가할 입력 매핑. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trigger")
	TObjectPtr<UInputMappingContext> LeftContext;

	/** 왼손 트리거 입력. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trigger")
	TObjectPtr<UInputAction> LeftFireAction;

	/** 오른손으로 잡았을 때 추가할 입력 매핑. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trigger")
	TObjectPtr<UInputMappingContext> RightContext;

	/** 오른손 트리거 입력. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Trigger")
	TObjectPtr<UInputAction> RightFireAction;

private:
	void HandleFire();

	UFUNCTION()
	void HandleFired(float Recoil);

	/** 지금 입력이 켜진 손. */
	EControllerHand ActiveHand = EControllerHand::Right;

	/** 지금 켜져 있는 입력 매핑. 꺼져 있으면 None. */
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Trigger|State")
	TObjectPtr<UInputMappingContext> ActiveContext;

	/** 트리거 바인딩. 끌 때 이 번호로 지운다. */
	uint32 BindingHandle = 0;
};
