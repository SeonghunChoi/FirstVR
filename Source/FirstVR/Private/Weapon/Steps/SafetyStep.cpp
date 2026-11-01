#include "Weapon/Steps/SafetyStep.h"

void USafetyStep::BeginPlay()
{
	Super::BeginPlay();

	bLocked = bInitiallyLocked;
}

void USafetyStep::Execute(FFireContext& Context)
{
	if (bLocked)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Safety on"), *GetName());
		Report(EStepResult::Failed);
		return;
	}

	Report(EStepResult::Success);
}

void USafetyStep::ToggleSafety()
{
	SetLocked(!bLocked);
}

void USafetyStep::SetLocked(bool bInLocked)
{
	if (bLocked == bInLocked)
	{
		return;
	}

	bLocked = bInLocked;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Safety %s"), *GetName(), bLocked ? TEXT("locked") : TEXT("released"));
	OnSafetyChanged(bLocked);
}
