#pragma once

#include "CoreMinimal.h"
#include "Weapon/Steps/WeaponStepComponent.h"
#include "GripStep.generated.h"

class USceneComponent;

/**
 * 파지 단계. 무기가 손에 잡혀 있지 않으면 Failed.
 * 잡기 자체는 잡기 시스템이 맡고, 이 단계는 결과만 SetHeld로 받는다.
 * 템플릿 잡기와의 연결은 블루프린트 자식(BP_GripStep)이 한다.
 */
UCLASS(Abstract)
class FIRSTVR_API UGripStep : public UWeaponStepComponent
{
	GENERATED_BODY()

public:
	virtual void Execute(FFireContext& Context) override;

	/** 파지 상태를 바꾼다. 잡기 시스템의 잡힘/놓임 이벤트에서 부른다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Grip")
	void SetHeld(bool bInHeld);

	/** 같은 액터에서 이름이 GripComponentName인 Scene Component를 찾는다. 없으면 None. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Grip")
	USceneComponent* FindGripComponent() const;

protected:
	/** 이 단계가 지켜볼 잡기 컴포넌트의 이름. 잡기 컴포넌트가 곧 파지 위치다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Grip")
	FName GripComponentName = TEXT("Grip");

private:
	/** 지금 손에 잡혀 있는지. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Grip|State", meta = (AllowPrivateAccess = "true"))
	bool bHeld = false;
};
