#include "Weapon/WeaponBase.h"

#include "Weapon/WeaponDataAsset.h"
#include "Weapon/Steps/WeaponStepComponent.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
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
	UE_LOG(LogWeapon, Log, TEXT("[%s] FIRE (RecoilScale %.2f, DamageScale %.2f)"), *GetName(), Context.RecoilScale, Context.DamageScale);
}
