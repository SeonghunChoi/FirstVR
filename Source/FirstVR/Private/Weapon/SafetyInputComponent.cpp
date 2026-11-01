#include "Weapon/SafetyInputComponent.h"

#include "Weapon/Steps/SafetyStep.h"

void USafetyInputComponent::BeginPlay()
{
	Super::BeginPlay();

	SafetyStep = GetOwner()->FindComponentByClass<USafetyStep>();
	if (!SafetyStep)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] Owner has no SafetyStep"), *GetName());
	}
}

void USafetyInputComponent::HandleAction()
{
	if (SafetyStep)
	{
		SafetyStep->ToggleSafety();
	}
}
