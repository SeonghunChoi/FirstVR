#include "Enemy/HitReaction.h"

#include "Components/SkeletalMeshComponent.h"
#include "Enemy/DamageReceiver.h"
#include "Weapon/AmmoDataAsset.h"

UHitReaction::UHitReaction()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PhysicalAnimationData.bIsLocalSimulation = false;
	PhysicalAnimationData.OrientationStrength = 1000.f;
	PhysicalAnimationData.AngularVelocityStrength = 100.f;
	PhysicalAnimationData.PositionStrength = 1000.f;
	PhysicalAnimationData.VelocityStrength = 100.f;
}

void UHitReaction::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	PhysicalAnimation = Owner->FindComponentByClass<UPhysicalAnimationComponent>();
	if (!Mesh || !PhysicalAnimation)
	{
		UE_LOG(LogEnemy, Error, TEXT("[%s] HitReaction needs a SkeletalMeshComponent and a PhysicalAnimationComponent"), *GetNameSafe(Owner));
		return;
	}
	PhysicalAnimation->SetSkeletalMeshComponent(Mesh);
	// 루트 바디(골반)가 물리로 돌 때 컴포넌트가 따라가면 월드 기준 목표 자세도 함께 움직여 골반이 돌아오지 않는다.
	Mesh->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::ComponentTransformIsKinematic;

	UDamageReceiver* Receiver = Owner->FindComponentByClass<UDamageReceiver>();
	if (!Receiver)
	{
		UE_LOG(LogEnemy, Error, TEXT("[%s] HitReaction needs a DamageReceiver"), *GetNameSafe(Owner));
		return;
	}
	Receiver->OnDamaged.AddDynamic(this, &UHitReaction::HandleDamaged);
	Receiver->OnDied.AddDynamic(this, &UHitReaction::HandleDied);
}

void UHitReaction::HandleDamaged(FName BoneName, float Damage, const FHitResult& Hit, UAmmoDataAsset* Ammo)
{
	if (!Ammo || Ammo->BaseDamage <= 0.f)
	{
		return;
	}

	FVector Direction = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = -Hit.ImpactNormal;
	}

	const float Strength = Ammo->ImpactImpulse * (Damage / Ammo->BaseDamage);
	ApplyPhysicalHit(BoneName, Direction * Strength);
}

void UHitReaction::ApplyPhysicalHit(FName BoneName, FVector Impulse)
{
	if (!Mesh || !PhysicalAnimation || BoneName.IsNone() || bRagdoll)
	{
		return;
	}

	FName RootBone = BoneName;
	for (const TPair<FName, float>& Reaction : ActiveReactions)
	{
		if (Reaction.Key == BoneName || Mesh->BoneIsChildOf(BoneName, Reaction.Key))
		{
			RootBone = Reaction.Key;
			break;
		}
	}
	for (auto It = ActiveReactions.CreateIterator(); It; ++It)
	{
		if (Mesh->BoneIsChildOf(It.Key(), RootBone))
		{
			It.RemoveCurrent();
		}
	}

	PhysicalAnimation->ApplyPhysicalAnimationSettingsBelow(RootBone, PhysicalAnimationData);
	Mesh->SetAllBodiesBelowSimulatePhysics(RootBone, true);
	Mesh->SetAllBodiesBelowPhysicsBlendWeight(RootBone, 1.f);
	Mesh->AddImpulse(Impulse, BoneName, true);

	ActiveReactions.Add(RootBone, 0.f);
	SetComponentTickEnabled(true);

	UE_LOG(LogEnemy, Log, TEXT("[%s] Physical hit %s (below %s), impulse %.0f"), *GetNameSafe(GetOwner()), *BoneName.ToString(), *RootBone.ToString(), Impulse.Size());
}

void UHitReaction::HandleDied()
{
	StartRagdoll();
}

void UHitReaction::StartRagdoll()
{
	if (!Mesh || !PhysicalAnimation || bRagdoll)
	{
		return;
	}
	bRagdoll = true;

	ActiveReactions.Empty();
	SetComponentTickEnabled(false);

	PhysicalAnimation->SetStrengthMultiplyer(0.f);
	Mesh->SetAllBodiesSimulatePhysics(true);
	Mesh->SetAllBodiesPhysicsBlendWeight(1.f);
	Mesh->WakeAllRigidBodies();

	UE_LOG(LogEnemy, Log, TEXT("[%s] Ragdoll"), *GetNameSafe(GetOwner()));
}

void UHitReaction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<FName> Finished;
	for (TPair<FName, float>& Reaction : ActiveReactions)
	{
		Reaction.Value += DeltaTime;
		const float Weight = 1.f - FMath::Clamp(Reaction.Value / RecoverTime, 0.f, 1.f);
		Mesh->SetAllBodiesBelowPhysicsBlendWeight(Reaction.Key, Weight);
		if (Weight <= 0.f)
		{
			Finished.Add(Reaction.Key);
		}
	}

	for (const FName& RootBone : Finished)
	{
		StopReaction(RootBone);
	}

	if (ActiveReactions.IsEmpty())
	{
		SetComponentTickEnabled(false);
	}
}

void UHitReaction::StopReaction(FName RootBone)
{
	Mesh->SetAllBodiesBelowSimulatePhysics(RootBone, false);
	ActiveReactions.Remove(RootBone);

	UE_LOG(LogEnemy, Log, TEXT("[%s] Recovered %s"), *GetNameSafe(GetOwner()), *RootBone.ToString());
}
