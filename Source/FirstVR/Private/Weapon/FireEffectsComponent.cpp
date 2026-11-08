#include "Weapon/FireEffectsComponent.h"

#include "Common/EffectSpawn.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"   // UFXSystemAsset (GetNameSafe에 완전한 타입이 필요)
#include "Sound/SoundBase.h"
#include "Weapon/WeaponBase.h"
#include "Weapon/WeaponTypes.h"

UFireEffectsComponent::UFireEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFireEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	Muzzle = FindSceneComponentByName(Owner, MuzzleComponentName);
	ShellEject = FindSceneComponentByName(Owner, ShellEjectComponentName);
	if (!Muzzle)
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] Fire effects found no component named %s"), *GetName(), *MuzzleComponentName.ToString());
	}

	if (AWeaponBase* Weapon = Cast<AWeaponBase>(GetOwner()))
	{
		Weapon->OnFired.AddDynamic(this, &UFireEffectsComponent::HandleFired);
	}
	else
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] Fire effects need an AWeaponBase owner"), *GetName());
	}
}

void UFireEffectsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AWeaponBase* Weapon = Cast<AWeaponBase>(GetOwner()))
	{
		Weapon->OnFired.RemoveDynamic(this, &UFireEffectsComponent::HandleFired);
	}
	Super::EndPlay(EndPlayReason);
}

void UFireEffectsComponent::HandleFired(float Recoil)
{
	PlayFireEffects();
}

void UFireEffectsComponent::PlayFireEffects()
{
	FirstVRFX::SpawnAttached(MuzzleEffect, Muzzle, MuzzleEffectScale);
	FirstVRFX::SpawnAttached(ShellEjectEffect, ShellEject);

	if (FireSound)
	{
		const FVector Location = Muzzle ? Muzzle->GetComponentLocation() : GetOwner()->GetActorLocation();
		const float Pitch = FMath::FRandRange(FireSoundPitchRange.X, FireSoundPitchRange.Y);
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Location, FireSoundVolume, Pitch);
	}

	UE_LOG(LogWeapon, Verbose, TEXT("[%s] Fire effects (muzzle %s, shell %s, sound %s)"), *GetName(),
		*GetNameSafe(MuzzleEffect), ShellEject ? *GetNameSafe(ShellEjectEffect) : TEXT("no point"), *GetNameSafe(FireSound));
}
