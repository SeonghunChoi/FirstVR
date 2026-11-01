#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInputComponent.h"
#include "SafetyInputComponent.generated.h"

class USafetyStep;

/**
 * 안전장치 입력. 잡은 손의 버튼(A/X)이 눌리면 같은 액터의 USafetyStep을 토글한다.
 * 잠김 상태와 피드백은 USafetyStep이 맡는다. 잡은 손의 입력 켜고 끄기는 부모 UWeaponInputComponent가 한다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API USafetyInputComponent : public UWeaponInputComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void HandleAction() override;

private:
	/** 토글할 안전장치 단계. BeginPlay에서 같은 액터에서 찾는다. */
	UPROPERTY()
	TObjectPtr<USafetyStep> SafetyStep;
};
