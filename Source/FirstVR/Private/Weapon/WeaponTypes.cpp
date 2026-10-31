#include "Weapon/WeaponTypes.h"

#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogWeapon);

USceneComponent* FindSceneComponentByName(const AActor* Owner, FName Name)
{
	if (!Owner)
	{
		return nullptr;
	}

	TArray<USceneComponent*> Components;
	Owner->GetComponents<USceneComponent>(Components);

	for (USceneComponent* Component : Components)
	{
		if (Component->GetFName() == Name)
		{
			return Component;
		}
	}

	return nullptr;
}
