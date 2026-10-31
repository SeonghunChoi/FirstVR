#include "Weapon/WeaponBase.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Weapon/AmmoDataAsset.h"
#include "Weapon/WeaponDataAsset.h"
#include "Weapon/WeaponProjectile.h"
#include "Weapon/Steps/WeaponStepComponent.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	Body->SetCollisionProfileName(TEXT("PhysicsActor"));
	SetRootComponent(Body);

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(Body);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	BuildSteps();
}

void AWeaponBase::BuildSteps()
{
	Steps.Reset();

	if (!WeaponData)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] WeaponData is not set"), *GetName());
		return;
	}

	for (const TSubclassOf<UWeaponStepComponent>& StepClass : WeaponData->FireSequence)
	{
		UWeaponStepComponent* Step = StepClass ? Cast<UWeaponStepComponent>(FindComponentByClass(StepClass)) : nullptr;
		if (!Step)
		{
			UE_LOG(LogWeapon, Error, TEXT("[%s] No component for step %s. Weapon cannot fire."), *GetName(), *GetNameSafe(StepClass.Get()));
			Steps.Reset();
			return;
		}
		Steps.Add(Step);
	}

	UE_LOG(LogWeapon, Log, TEXT("[%s] %d steps ready"), *GetName(), Steps.Num());
}

void AWeaponBase::RequestFire()
{
	if (IsFiring())
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Fire ignored: already firing"), *GetName());
		return;
	}

	if (Steps.IsEmpty())
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] Fire ignored: no steps"), *GetName());
		return;
	}

	const double Interval = 1.0 / WeaponData->FireRate;
	if (GetWorld()->GetTimeSeconds() - LastFireTime < Interval)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Fire ignored: cooldown"), *GetName());
		return;
	}

	UE_LOG(LogWeapon, Log, TEXT("[%s] Fire requested"), *GetName());
	Context = FFireContext();
	RunStep(0);
}

void AWeaponBase::RunStep(int32 Index)
{
	CurrentStepIndex = Index;
	UWeaponStepComponent* Step = Steps[Index];
	Step->OnStepResult.AddUniqueDynamic(this, &AWeaponBase::HandleStepResult);
	Step->Execute(Context);
}

void AWeaponBase::HandleStepResult(EStepResult Result)
{
	if (!Steps.IsValidIndex(CurrentStepIndex))
	{
		return;
	}

	if (Result == EStepResult::Pending)
	{
		return;
	}

	UWeaponStepComponent* Step = Steps[CurrentStepIndex];
	Step->OnStepResult.RemoveDynamic(this, &AWeaponBase::HandleStepResult);

	if (Result == EStepResult::Failed)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Fire aborted at step %d/%d"), *GetName(), CurrentStepIndex + 1, Steps.Num());
		CurrentStepIndex = INDEX_NONE;
		return;
	}

	const int32 NextIndex = CurrentStepIndex + 1;
	if (Steps.IsValidIndex(NextIndex))
	{
		RunStep(NextIndex);
		return;
	}

	CurrentStepIndex = INDEX_NONE;
	Fire();
}

void AWeaponBase::Fire()
{
	LastFireTime = GetWorld()->GetTimeSeconds();

	const float Recoil = WeaponData->Recoil * Context.RecoilScale;
	UE_LOG(LogWeapon, Log, TEXT("[%s] FIRE (Ammo %s, Recoil %.2f, DamageScale %.2f)"), *GetName(), *GetNameSafe(Context.Ammo), Recoil, Context.DamageScale);

	if (!Context.Ammo)
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] No ammo in context. Is a magazine step in FireSequence?"), *GetName());
		return;
	}

	if (!Context.Ammo->ProjectileClass)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] %s has no ProjectileClass"), *GetName(), *Context.Ammo->GetName());
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeaponProjectile* Projectile = GetWorld()->SpawnActor<AWeaponProjectile>(Context.Ammo->ProjectileClass, Muzzle->GetComponentTransform(), Params);
	if (Projectile)
	{
		Projectile->Launch(Context.Ammo, Context.DamageScale);
		OnFired.Broadcast(Recoil);
	}
}
