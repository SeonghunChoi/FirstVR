#pragma once

#include "CoreMinimal.h"
#include "Weapon/Steps/WeaponStepComponent.h"
#include "TwoHandGripStep.generated.h"

class USceneComponent;

/**
 * 두 손 파지 단계. 발사를 막지 않고 항상 Success이며, 두 손이면 TwoHandRecoilScale, 한 손이면 OneHandRecoilScale을 RecoilScale에 곱한다.
 * 주 손과 보조 손은 SetMainHand/SetSupportHand로 받는다(놓이면 None). 보조 손잡이는 무기를 손에 붙이지 않는 잡기다.
 * 두 손일 때만 틱을 켜고, 주 손잡이를 축으로 무기를 돌려 주 손잡이 → 보조 손잡이 방향이 보조 손을 향하게 한다.
 * 템플릿 잡기와의 연결은 블루프린트 자식(BP_TwoHandGripStep)이 한다.
 */
UCLASS(Abstract)
class FIRSTVR_API UTwoHandGripStep : public UWeaponStepComponent
{
	GENERATED_BODY()

public:
	UTwoHandGripStep();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void Execute(FFireContext& Context) override;

	/** 주 손잡이를 잡은 손. 놓이면 None을 넘긴다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|TwoHand")
	void SetMainHand(USceneComponent* Hand);

	/** 보조 손잡이를 잡은 손. 놓이면 None을 넘긴다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|TwoHand")
	void SetSupportHand(USceneComponent* Hand);

	/** 주 손잡이와 보조 손잡이가 모두 잡혀 있는지. */
	UFUNCTION(BlueprintPure, Category = "Weapon|TwoHand")
	bool IsTwoHanded() const { return MainHand && SupportHand; }

	/** 같은 액터에서 이름이 GripComponentName인 Scene Component를 찾는다. 없으면 None. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|TwoHand")
	USceneComponent* FindGripComponent() const;

	/** 같은 액터에서 이름이 SupportComponentName인 Scene Component를 찾는다. 없으면 None. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|TwoHand")
	USceneComponent* FindSupportComponent() const;

protected:
	/** 주 손잡이 잡기 컴포넌트의 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|TwoHand")
	FName GripComponentName = TEXT("Grip");

	/** 보조 손잡이 잡기 컴포넌트의 이름. 무기를 손에 붙이지 않는 잡기 종류(Custom)여야 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|TwoHand")
	FName SupportComponentName = TEXT("SupportGrip");

	/** 한 손으로 쏠 때 RecoilScale에 곱하는 값. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|TwoHand", meta = (ClampMin = "0.0"))
	float OneHandRecoilScale = 1.f;

	/** 두 손으로 쏠 때 RecoilScale에 곱하는 값. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|TwoHand", meta = (ClampMin = "0.0"))
	float TwoHandRecoilScale = 0.4f;

private:
	USceneComponent* FindNamedComponent(FName Name) const;

	/** 두 손 여부가 바뀌었으면 조준을 시작하거나 끝낸다. */
	void UpdateTwoHandAim();
	void BeginTwoHandAim();
	void EndTwoHandAim();

	/** 두 손 조준 중인지. 조준 중에만 틱이 켜진다. */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Weapon|TwoHand|State")
	bool bAiming = false;

	/** 조준을 시작할 때 무기 루트가 붙어 있던 부모(주 손)와 그때의 상대 트랜스폼. 매 틱 이 한 손 자세에서 출발해 돌린다. */
	TWeakObjectPtr<USceneComponent> AimParent;
	FTransform AimBaseRelative;

	/** 액터 기준 주 손잡이·보조 손잡이 위치. 조준을 시작할 때 구한다. */
	FVector GripLocal = FVector::ZeroVector;
	FVector SupportLocal = FVector::ZeroVector;

	/** 주 손잡이를 잡은 손. */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Weapon|TwoHand|State")
	TObjectPtr<USceneComponent> MainHand;

	/** 보조 손잡이를 잡은 손. */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Weapon|TwoHand|State")
	TObjectPtr<USceneComponent> SupportHand;
};
