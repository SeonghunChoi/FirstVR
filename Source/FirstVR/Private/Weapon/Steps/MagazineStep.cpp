#include "Weapon/Steps/MagazineStep.h"

#include "Engine/World.h"
#include "Weapon/AmmoDataAsset.h"
#include "Weapon/Magazine.h"

void UMagazineStep::BeginPlay()
{
	Super::BeginPlay();

	if (!InitialMagazine)
	{
		return;
	}

	const USceneComponent* Well = FindMagazineWell();
	if (!Well)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] InitialMagazine is set but '%s' was not found on %s"), *GetName(), *MagazineWellName.ToString(), *GetOwner()->GetName());
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMagazine* Magazine = GetWorld()->SpawnActor<AMagazine>(InitialMagazine, Well->GetComponentTransform(), Params);
	if (Magazine && !InsertMagazine(Magazine))
	{
		// 맞지 않는 탄창을 지정했으면 버린다. 이유는 InsertMagazine이 로그로 남긴다.
		Magazine->Destroy();
	}
}

void UMagazineStep::Execute(FFireContext& Context)
{
	if (!InsertedMagazine)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] No magazine"), *GetName());
		Report(EStepResult::Failed);
		return;
	}

	UAmmoDataAsset* Ammo = InsertedMagazine->TakeRound();
	if (!Ammo)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Empty"), *GetName());
		Report(EStepResult::Failed);
		return;
	}

	Context.Ammo = Ammo;
	UE_LOG(LogWeapon, Log, TEXT("[%s] %s, %d/%d left"), *GetName(), *Ammo->GetName(), InsertedMagazine->GetRounds(), InsertedMagazine->GetCapacity());
	Report(EStepResult::Success);
}

bool UMagazineStep::CanAcceptMagazine(const AMagazine* Magazine) const
{
	if (!Magazine)
	{
		return false;
	}

	for (const TSubclassOf<AMagazine>& Allowed : AllowedMagazines)
	{
		if (Allowed && Magazine->IsA(Allowed))
		{
			return true;
		}
	}
	return false;
}

bool UMagazineStep::InsertMagazine(AMagazine* Magazine)
{
	if (!Magazine || Magazine->IsInserted())
	{
		return false;
	}

	if (InsertedMagazine)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Insert rejected: %s is already inserted"), *GetName(), *InsertedMagazine->GetName());
		return false;
	}

	if (!CanAcceptMagazine(Magazine))
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Insert rejected: %s does not fit"), *GetName(), *Magazine->GetClass()->GetName());
		return false;
	}

	USceneComponent* Well = FindMagazineWell();
	if (!Well)
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] Insert rejected: '%s' not found"), *GetName(), *MagazineWellName.ToString());
		return false;
	}

	InsertedMagazine = Magazine;
	Magazine->AttachToWell(this, Well);
	OnMagazineChanged(Magazine);
	return true;
}

AMagazine* UMagazineStep::EjectMagazine()
{
	AMagazine* Magazine = InsertedMagazine;
	if (!Magazine)
	{
		return nullptr;
	}

	InsertedMagazine = nullptr;
	Magazine->DetachFromWell(true);
	OnMagazineChanged(nullptr);
	return Magazine;
}

void UMagazineStep::ReleaseMagazine(AMagazine* Magazine)
{
	if (!Magazine || Magazine != InsertedMagazine)
	{
		return;
	}

	InsertedMagazine = nullptr;
	OnMagazineChanged(nullptr);
}

USceneComponent* UMagazineStep::FindMagazineWell() const
{
	return FindSceneComponentByName(GetOwner(), MagazineWellName);
}
