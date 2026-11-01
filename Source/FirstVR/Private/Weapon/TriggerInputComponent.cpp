#include "Weapon/TriggerInputComponent.h"

#include "GameFramework/PlayerController.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponBase.h"

void UTriggerInputComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AWeaponBase* Weapon = Cast<AWeaponBase>(GetOwner()))
	{
		Weapon->OnFired.AddDynamic(this, &UTriggerInputComponent::HandleFired);
	}
}

void UTriggerInputComponent::HandleAction()
{
	if (AWeaponBase* Weapon = Cast<AWeaponBase>(GetOwner()))
	{
		Weapon->RequestFire();
	}
}

void UTriggerInputComponent::HandleFired(float Recoil)
{
	if (!IsHandInputEnabled())
	{
		return;
	}

	if (!FireHaptic)
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] FireHaptic is not set"), *GetName());
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	const EControllerHand Hand = GetActiveHand();
	const float Scale = FMath::Clamp(Recoil, 0.f, 1.f);
	PlayerController->PlayHapticEffect(FireHaptic, Hand, Scale);
	UE_LOG(LogWeapon, Log, TEXT("[%s] Fire haptic %.2f (%s hand)"), *GetName(), Scale, Hand == EControllerHand::Right ? TEXT("right") : TEXT("left"));
}
