#include "Weapon/Steps/GripStep.h"

#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

void UGripStep::Execute(FFireContext& Context)
{
	if (!bHeld)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Not held"), *GetName());
		Report(EStepResult::Failed);
		return;
	}

	Report(EStepResult::Success);
}

void UGripStep::SetHeld(bool bInHeld)
{
	if (bHeld == bInHeld)
	{
		return;
	}

	bHeld = bInHeld;
	UE_LOG(LogWeapon, Log, TEXT("[%s] %s"), *GetName(), bHeld ? TEXT("Held") : TEXT("Released"));
}

USceneComponent* UGripStep::FindGripComponent() const
{
	TArray<USceneComponent*> Components;
	GetOwner()->GetComponents<USceneComponent>(Components);

	for (USceneComponent* Component : Components)
	{
		if (Component->GetFName() == GripComponentName)
		{
			return Component;
		}
	}

	UE_LOG(LogWeapon, Error, TEXT("[%s] No component named %s"), *GetName(), *GripComponentName.ToString());
	return nullptr;
}
