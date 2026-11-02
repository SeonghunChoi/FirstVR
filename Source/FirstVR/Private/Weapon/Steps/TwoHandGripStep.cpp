#include "Weapon/Steps/TwoHandGripStep.h"

#include "Components/SceneComponent.h"

void UTwoHandGripStep::Execute(FFireContext& Context)
{
	const bool bTwoHanded = IsTwoHanded();
	const float Scale = bTwoHanded ? TwoHandRecoilScale : OneHandRecoilScale;
	Context.RecoilScale *= Scale;

	UE_LOG(LogWeapon, Log, TEXT("[%s] %s, recoil scale x%.2f"), *GetName(), bTwoHanded ? TEXT("Two hands") : TEXT("One hand"), Scale);
	Report(EStepResult::Success);
}

void UTwoHandGripStep::SetMainHand(USceneComponent* Hand)
{
	if (MainHand == Hand)
	{
		return;
	}

	MainHand = Hand;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Main %s (%s)"), *GetName(), Hand ? TEXT("held") : TEXT("released"), *GetNameSafe(Hand));
}

void UTwoHandGripStep::SetSupportHand(USceneComponent* Hand)
{
	if (SupportHand == Hand)
	{
		return;
	}

	SupportHand = Hand;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Support %s (%s)"), *GetName(), Hand ? TEXT("held") : TEXT("released"), *GetNameSafe(Hand));
}

USceneComponent* UTwoHandGripStep::FindGripComponent() const
{
	return FindNamedComponent(GripComponentName);
}

USceneComponent* UTwoHandGripStep::FindSupportComponent() const
{
	return FindNamedComponent(SupportComponentName);
}

USceneComponent* UTwoHandGripStep::FindNamedComponent(FName Name) const
{
	USceneComponent* Component = FindSceneComponentByName(GetOwner(), Name);
	if (!Component)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] No component named %s"), *GetName(), *Name.ToString());
	}
	return Component;
}
