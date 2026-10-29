#include "Weapon/Steps/DebugSteps.h"

#include "Engine/World.h"
#include "TimerManager.h"

void UDebugSuccessStep::Execute(FFireContext& Context)
{
	Report(EStepResult::Success);
}

void UDebugFailStep::Execute(FFireContext& Context)
{
	Report(EStepResult::Failed);
}

void UDebugPendingStep::Execute(FFireContext& Context)
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDebugPendingStep::Finish, Delay, false);
	Report(EStepResult::Pending);
}

void UDebugPendingStep::Finish()
{
	Report(EStepResult::Success);
}
