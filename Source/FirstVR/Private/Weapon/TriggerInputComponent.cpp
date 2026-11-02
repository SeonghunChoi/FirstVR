#include "Weapon/TriggerInputComponent.h"

#include "GameFramework/PlayerController.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponBase.h"

UTriggerInputComponent::UTriggerInputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTriggerInputComponent::BeginPlay()
{
	Super::BeginPlay();

	if (FireModes.IsEmpty())
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] FireModes is empty, using Single"), *GetName());
		FireModes.Add(EFireMode::Single);
	}
	FireMode = FireModes[0];

	if (AWeaponBase* Weapon = Cast<AWeaponBase>(GetOwner()))
	{
		Weapon->OnFired.AddDynamic(this, &UTriggerInputComponent::HandleFired);
		Weapon->OnFireAborted.AddDynamic(this, &UTriggerInputComponent::HandleFireAborted);
	}
}

void UTriggerInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AWeaponBase* Weapon = Cast<AWeaponBase>(GetOwner());
	if (Weapon && Weapon->CanRequestFire())
	{
		Weapon->RequestFire();
	}
}

bool UTriggerInputComponent::SetFireMode(EFireMode NewMode)
{
	if (!FireModes.Contains(NewMode))
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] Fire mode %s is not available"), *GetName(), *UEnum::GetValueAsString(NewMode));
		return false;
	}

	if (FireMode == NewMode)
	{
		return true;
	}

	FireMode = NewMode;
	if (FireMode != EFireMode::Auto)
	{
		StopAutoFire(TEXT("mode changed"));
	}

	UE_LOG(LogWeapon, Log, TEXT("[%s] Fire mode %s"), *GetName(), *UEnum::GetValueAsString(FireMode));
	OnFireModeChanged.Broadcast(FireMode);
	return true;
}

void UTriggerInputComponent::HandleAction()
{
	AWeaponBase* Weapon = Cast<AWeaponBase>(GetOwner());
	if (!Weapon)
	{
		return;
	}

	if (FireMode == EFireMode::Auto)
	{
		bAutoFiring = true;
		SetComponentTickEnabled(true);
		UE_LOG(LogWeapon, Log, TEXT("[%s] Auto fire started"), *GetName());
	}

	Weapon->RequestFire();
}

void UTriggerInputComponent::HandleActionReleased()
{
	StopAutoFire(TEXT("trigger released"));
}

void UTriggerInputComponent::StopAutoFire(const TCHAR* Reason)
{
	if (!bAutoFiring)
	{
		return;
	}

	bAutoFiring = false;
	SetComponentTickEnabled(false);
	UE_LOG(LogWeapon, Log, TEXT("[%s] Auto fire stopped (%s)"), *GetName(), Reason);
}

void UTriggerInputComponent::HandleFireAborted()
{
	StopAutoFire(TEXT("fire aborted"));
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
