#pragma once

#include "CoreMinimal.h"
#include "Weapon/Steps/WeaponStepComponent.h"
#include "SafetyStep.generated.h"

/**
 * 안전장치 단계. 잠겨 있으면 Failed, 풀려 있으면 Success.
 * 잠금 상태는 SetLocked/ToggleSafety로만 바꾼다. 버튼 입력은 입력 컴포넌트가 받아 이 함수들을 부른다.
 */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API USafetyStep : public UWeaponStepComponent
{
	GENERATED_BODY()

public:
	virtual void Execute(FFireContext& Context) override;

	/** 잠금 상태를 뒤집는다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Safety")
	void ToggleSafety();

	/** 잠금 상태를 정한다. 바뀌었을 때만 OnSafetyChanged를 부른다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Safety")
	void SetLocked(bool bInLocked);

	UFUNCTION(BlueprintPure, Category = "Weapon|Safety")
	bool IsLocked() const { return bLocked; }

protected:
	virtual void BeginPlay() override;

	/** 걸거나 풀었을 때의 피드백(딸깍 소리, 표시 등). 블루프린트 자식이 구현한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Safety")
	void OnSafetyChanged(bool bNewLocked);

	/** 배치별 처음 잠금 상태. BeginPlay에서 bLocked로 옮긴다. */
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Safety")
	bool bInitiallyLocked = true;

private:
	/** 지금 잠겨 있는지. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Safety|State", meta = (AllowPrivateAccess = "true"))
	bool bLocked = true;
};
