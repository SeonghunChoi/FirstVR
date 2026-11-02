#include "Weapon/SelectorInputComponent.h"

#include "Weapon/TriggerInputComponent.h"
#include "Weapon/Steps/SafetyStep.h"

void USelectorInputComponent::BeginPlay()
{
	Super::BeginPlay();

	SafetyStep = GetOwner()->FindComponentByClass<USafetyStep>();
	Trigger = GetOwner()->FindComponentByClass<UTriggerInputComponent>();
	if (!Trigger)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] Owner has no TriggerInputComponent"), *GetName());
	}
}

void USelectorInputComponent::HandleAction()
{
	if (!Trigger)
	{
		return;
	}

	const TArray<EFireMode>& Modes = Trigger->GetFireModes();
	const int32 SafeCount = SafetyStep ? 1 : 0;
	const int32 Count = SafeCount + Modes.Num();
	if (Count == 0)
	{
		return;
	}

	const bool bSafe = SafetyStep && SafetyStep->IsLocked();
	const int32 Current = bSafe ? 0 : SafeCount + Modes.IndexOfByKey(Trigger->GetFireMode());
	const int32 Next = (Current + 1) % Count;
	const bool bNextSafe = Next < SafeCount;

	if (!bNextSafe)
	{
		Trigger->SetFireMode(Modes[Next - SafeCount]);
	}
	if (SafetyStep)
	{
		SafetyStep->SetLocked(bNextSafe);
	}

	UE_LOG(LogWeapon, Log, TEXT("[%s] Selector %s"), *GetName(), bNextSafe ? TEXT("Safe") : *UEnum::GetValueAsString(Trigger->GetFireMode()));
}
