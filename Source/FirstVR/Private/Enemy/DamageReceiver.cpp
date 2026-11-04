#include "Enemy/DamageReceiver.h"

#include "Enemy/EnemyDataAsset.h"
#include "UObject/UObjectIterator.h"
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
	OnDamaged.Broadcast(BoneName, Damage, Hit, Ammo);

	if (EnemyData->IsLegBone(BoneName))
	{
		LegDamage += Damage;
		UE_LOG(LogEnemy, Log, TEXT("[%s] Leg damage %.1f/%.1f"), *GetNameSafe(GetOwner()), LegDamage, EnemyData->LimpThreshold);
	}

	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	else if (!bLimping && LegDamage >= EnemyData->LimpThreshold && LegDamage > 0.f)
	{
		StartLimp(EnemyData->GetLegSide(BoneName));
	}
}

void UDamageReceiver::StartLimp(ELegSide Leg)
{
	if (bLimping || bDead)
	{
		return;
	}

	bLimping = true;
	InjuredLeg = Leg;
	UE_LOG(LogEnemy, Log, TEXT("[%s] Limp started (%s leg)"), *GetNameSafe(GetOwner()), *UEnum::GetDisplayValueAsText(InjuredLeg).ToString());
	OnLimpStarted.Broadcast(InjuredLeg);
}

void UDamageReceiver::Kill()
{
	if (bDead || !EnemyData)
	{
		return;
	}

	CurrentHealth = 0.f;
	UE_LOG(LogEnemy, Log, TEXT("[%s] Killed by command"), *GetNameSafe(GetOwner()));
	Die();
}

void UDamageReceiver::Die()
{
	bDead = true;
	UE_LOG(LogEnemy, Log, TEXT("[%s] Died, corpse life span %.1fs"), *GetNameSafe(GetOwner()), EnemyData->CorpseLifeSpan);
	GetOwner()->SetLifeSpan(EnemyData->CorpseLifeSpan);
	OnDied.Broadcast();
}

/** 플레이 중인 월드의 살아 있는 적마다 Action을 부른다. */
static void ForEachEnemyInPlay(TFunctionRef<void(UDamageReceiver&)> Action)
{
	for (TObjectIterator<UDamageReceiver> It; It; ++It)
	{
		const UWorld* World = It->GetWorld();
		if (World && World->IsGameWorld() && It->HasBegunPlay())
		{
			Action(**It);
		}
	}
}

static FAutoConsoleCommand EnemyLimpCommand(
	TEXT("Enemy.Limp"),
	TEXT("Makes every living enemy in play limp. Usage: Enemy.Limp [Left|Right] (default Right)"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		const ELegSide Leg = Args.Num() > 0 && Args[0].Equals(TEXT("Left"), ESearchCase::IgnoreCase) ? ELegSide::Left : ELegSide::Right;
		ForEachEnemyInPlay([Leg](UDamageReceiver& Receiver) { Receiver.StartLimp(Leg); });
	}));

static FAutoConsoleCommand EnemyKillCommand(
	TEXT("Enemy.Kill"),
	TEXT("Kills every living enemy in play."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		ForEachEnemyInPlay([](UDamageReceiver& Receiver) { Receiver.Kill(); });
	}));
