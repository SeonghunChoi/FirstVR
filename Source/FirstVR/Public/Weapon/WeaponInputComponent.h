#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "WeaponInputComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class USceneComponent;

/**
 * 무기 입력 컴포넌트의 공통 부모. 무기가 잡히면 잡은 손의 입력 매핑을 켜고 버튼을 바인딩하며, 놓이면 끈다.
 * 자식은 버튼이 눌렸을 때 할 일(HandleAction)만 구현한다. 발사 흐름의 단계가 아니다.
 * 어느 손인지는 블루프린트 자식이 템플릿 잡기 컴포넌트를 구독해 EnableForHand/DisableForHand로 알려 준다.
 * 무기 액터의 입력은 같은 액터의 입력 컴포넌트가 모두 꺼졌을 때만 끈다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API UWeaponInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponInputComponent();

	/** 잡은 손의 입력을 켠다. 이미 켜져 있으면 먼저 끈다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Input")
	void EnableForHand(bool bRightHand);

	/** 입력을 끈다. 같은 액터에 켜진 입력 컴포넌트가 하나도 없으면 무기 액터의 입력도 끈다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Input")
	void DisableForHand();

	/** 지금 입력이 켜져 있는지. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Input")
	bool IsHandInputEnabled() const { return ActiveContext != nullptr; }

	/** 같은 액터에서 이름이 GripComponentName인 Scene Component를 찾는다. 없으면 None. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Input")
	USceneComponent* FindGripComponent() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 버튼이 눌렸을 때(Started) 부른다. 자식이 할 일을 구현한다. */
	virtual void HandleAction() {}

	/** 지금 입력이 켜진 손. 꺼져 있으면 마지막으로 켜졌던 손. */
	EControllerHand GetActiveHand() const { return ActiveHand; }

	/** 이 입력을 켜고 끌 기준이 되는 잡기 컴포넌트의 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Input")
	FName GripComponentName = TEXT("Grip");

	/** 왼손으로 잡았을 때 추가할 입력 매핑. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Input")
	TObjectPtr<UInputMappingContext> LeftContext;

	/** 왼손 버튼 입력. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Input")
	TObjectPtr<UInputAction> LeftAction;

	/** 오른손으로 잡았을 때 추가할 입력 매핑. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Input")
	TObjectPtr<UInputMappingContext> RightContext;

	/** 오른손 버튼 입력. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Input")
	TObjectPtr<UInputAction> RightAction;

private:
	/** 같은 액터에 입력이 켜진 입력 컴포넌트가 있는지. */
	bool IsAnyWeaponInputEnabled() const;

	/** 지금 입력이 켜진 손. */
	EControllerHand ActiveHand = EControllerHand::Right;

	/** 지금 켜져 있는 입력 매핑. 꺼져 있으면 None. */
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Input|State")
	TObjectPtr<UInputMappingContext> ActiveContext;

	/** 버튼 바인딩. 끌 때 이 번호로 지운다. */
	uint32 BindingHandle = 0;
};
