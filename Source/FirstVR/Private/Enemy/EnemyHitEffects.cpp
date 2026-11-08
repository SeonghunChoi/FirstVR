#include "Enemy/EnemyHitEffects.h"

#include "Common/EffectSpawn.h"
#include "Enemy/DamageReceiver.h"
#include "Enemy/EnemyDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UEnemyHitEffects::UEnemyHitEffects()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyHitEffects::BeginPlay()
{
	Super::BeginPlay();

	DamageReceiver = GetOwner()->FindComponentByClass<UDamageReceiver>();
	if (DamageReceiver)
	{
		DamageReceiver->OnDamaged.AddDynamic(this, &UEnemyHitEffects::HandleDamaged);
	}
}

void UEnemyHitEffects::HandleDamaged(FName BoneName, float Damage, const FHitResult& Hit, UAmmoDataAsset* Ammo)
{
	const UEnemyDataAsset* Data = DamageReceiver ? DamageReceiver->GetEnemyData() : nullptr;
	if (!Data)
	{
		return;
	}

	const FVector Normal = Hit.ImpactNormal.IsNearlyZero() ? FVector::UpVector : FVector(Hit.ImpactNormal);
	FirstVRFX::SpawnAtLocation(this, Data->HitEffect, Hit.ImpactPoint, FRotationMatrix::MakeFromZ(Normal).Rotator(), Data->HitEffectScale);
	if (Data->HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Data->HitSound, Hit.ImpactPoint, Data->HitSoundVolume, FMath::FRandRange(0.9f, 1.1f));
	}
}
