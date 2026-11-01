#include "Enemy/DamageReceiver.h"

#include "Enemy/EnemyDataAsset.h"
#include "Weapon/AmmoDataAsset.h"

DEFINE_LOG_CATEGORY(LogEnemy);

UDamageReceiver::UDamageReceiver()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDamageReceiver::BeginPlay()
{
	Super::BeginPlay();

	if (!EnemyData)
	{
		UE_LOG(LogEnemy, Error, TEXT("[%s] EnemyData is not set"), *GetNameSafe(GetOwner()));
		return;
	}
	CurrentHealth = EnemyData->MaxHealth;
}

float UDamageReceiver::CalculateDamage(FName BoneName, const UAmmoDataAsset* Ammo, float DamageScale) const
{
	if (!EnemyData || !Ammo)
	{
		return 0.f;
	}

	const float ArmorFactor = EnemyData->Armor > 0.f ? FMath::Min(1.f, Ammo->Penetration / EnemyData->Armor) : 1.f;
	return Ammo->BaseDamage * EnemyData->GetBoneMultiplier(BoneName) * DamageScale * ArmorFactor;
}

void UDamageReceiver::ReceiveHit(const FHitResult& Hit, UAmmoDataAsset* Ammo, float DamageScale)
{
	if (bDead || !EnemyData)
	{
		return;
	}

	const FName BoneName = Hit.BoneName;
	const float Damage = CalculateDamage(BoneName, Ammo, DamageScale);
	CurrentHealth = FMath::Max(0.f, CurrentHealth - Damage);

	UE_LOG(LogEnemy, Log, TEXT("[%s] Hit %s by %s: damage %.1f (x%.2f), health %.1f/%.1f"),
		*GetNameSafe(GetOwner()), *BoneName.ToString(), *GetNameSafe(Ammo), Damage, EnemyData->GetBoneMultiplier(BoneName), CurrentHealth, EnemyData->MaxHealth);
	OnDamaged.Broadcast(BoneName, Damage, Hit);

	if (EnemyData->IsLegBone(BoneName))
	{
		LegDamage += Damage;
		UE_LOG(LogEnemy, Log, TEXT("[%s] Leg damage %.1f/%.1f"), *GetNameSafe(GetOwner()), LegDamage, EnemyData->LimpThreshold);
	}

	if (CurrentHealth <= 0.f)
	{
		bDead = true;
		UE_LOG(LogEnemy, Log, TEXT("[%s] Died"), *GetNameSafe(GetOwner()));
		OnDied.Broadcast();
	}
	else if (!bLimping && LegDamage >= EnemyData->LimpThreshold && LegDamage > 0.f)
	{
		bLimping = true;
		UE_LOG(LogEnemy, Log, TEXT("[%s] Limp started"), *GetNameSafe(GetOwner()));
		OnLimpStarted.Broadcast();
	}
}
