#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Weapon/Steps/WeaponStepComponent.h"
#include "DebugSteps.generated.h"

/** 검증용: 항상 Success를 보고한다. */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UDebugSuccessStep : public UWeaponStepComponent
{
	GENERATED_BODY()

public:
	virtual void Execute(FFireContext& Context) override;
};

/** 검증용: 항상 Failed를 보고한다. */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UDebugFailStep : public UWeaponStepComponent
{
	GENERATED_BODY()

public:
	virtual void Execute(FFireContext& Context) override;
};

/** 검증용: Pending을 보고하고, Delay초 뒤 Success를 보고한다. */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UDebugPendingStep : public UWeaponStepComponent
{
	GENERATED_BODY()

public:
	virtual void Execute(FFireContext& Context) override;

protected:
	/** Pending에서 Success까지 기다리는 시간(초). 타이머는 0 이하면 동작하지 않으므로 최소 0.1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Step", meta = (ClampMin = "0.1"))
	float Delay = 1.f;

private:
	void Finish();

	FTimerHandle TimerHandle;
};
