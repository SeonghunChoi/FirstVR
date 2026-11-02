#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInputComponent.h"
#include "SelectorInputComponent.generated.h"

class USafetyStep;
class UTriggerInputComponent;

/**
 * 조정간 입력. 잡은 손의 버튼(A/X)이 눌릴 때마다 안전 → 트리거의 발사 모드들(FireModes 순서) → 안전으로 돌린다.
 * 안전 위치는 같은 액터에 USafetyStep이 있을 때만 있다. 잠김은 USafetyStep, 발사 모드는 UTriggerInputComponent가 가진다.
 * 발사 모드가 단발뿐인 무기에서는 안전장치 입력(USafetyInputComponent)과 같게 동작한다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API USelectorInputComponent : public UWeaponInputComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void HandleAction() override;

private:
	/** 안전 위치에서 잠글 안전장치 단계. 없으면 안전 위치가 없다. */
	UPROPERTY()
	TObjectPtr<USafetyStep> SafetyStep;

	/** 발사 모드를 바꿀 트리거 입력. */
	UPROPERTY()
	TObjectPtr<UTriggerInputComponent> Trigger;
};
