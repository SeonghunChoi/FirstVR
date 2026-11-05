#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInputComponent.h"
#include "MagazineReleaseInputComponent.generated.h"

class UMagazineStep;

/**
 * 탄창 멈치 입력. 무기를 잡은 손의 버튼(B/Y)이 눌리면 같은 액터의 UMagazineStep::EjectMagazine으로 탄창을 빼서 떨어뜨린다.
 * 탄창 상태와 떨어뜨리는 처리는 UMagazineStep과 AMagazine이 맡는다. 잡은 손의 입력 켜고 끄기는 부모 UWeaponInputComponent가 한다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API UMagazineReleaseInputComponent : public UWeaponInputComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void HandleAction() override;

private:
	/** 탄창을 뺄 탄창 단계. BeginPlay에서 같은 액터에서 찾는다. */
	UPROPERTY()
	TObjectPtr<UMagazineStep> MagazineStep;
};
