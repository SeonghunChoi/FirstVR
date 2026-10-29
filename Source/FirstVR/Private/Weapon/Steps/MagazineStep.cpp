#include "Weapon/Steps/MagazineStep.h"

#include "Weapon/AmmoDataAsset.h"

void UMagazineStep::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = 0;
	LoadedAmmo = nullptr;

	if (InitialAmmo <= 0)
	{
		return;
	}

	if (AllowedAmmo.IsEmpty())
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] InitialAmmo is set but AllowedAmmo is empty"), *GetName());
		return;
	}

	AddAmmo(AllowedAmmo[0], InitialAmmo);
}

void UMagazineStep::Execute(FFireContext& Context)
{
	if (CurrentAmmo <= 0)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Empty"), *GetName());
		Report(EStepResult::Failed);
		return;
	}

	--CurrentAmmo;
	Context.Ammo = LoadedAmmo;
	UE_LOG(LogWeapon, Log, TEXT("[%s] %s, %d/%d left"), *GetName(), *GetNameSafe(LoadedAmmo), CurrentAmmo, Capacity);

	if (CurrentAmmo == 0)
	{
		LoadedAmmo = nullptr;
	}

	Report(EStepResult::Success);
}

int32 UMagazineStep::AddAmmo(UAmmoDataAsset* Ammo, int32 Count)
{
	if (!Ammo || Count <= 0)
	{
		return 0;
	}

	if (!AllowedAmmo.Contains(Ammo))
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] AddAmmo rejected: %s is not allowed"), *GetName(), *Ammo->GetName());
		return 0;
	}

	if (CurrentAmmo > 0 && Ammo != LoadedAmmo)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] AddAmmo rejected: %s loaded, %s given"), *GetName(), *GetNameSafe(LoadedAmmo), *Ammo->GetName());
		return 0;
	}

	const int32 Added = FMath::Min(Count, Capacity - CurrentAmmo);
	if (Added <= 0)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] AddAmmo rejected: full"), *GetName());
		return 0;
	}

	LoadedAmmo = Ammo;
	CurrentAmmo += Added;
	UE_LOG(LogWeapon, Log, TEXT("[%s] AddAmmo %s x%d, %d/%d"), *GetName(), *Ammo->GetName(), Added, CurrentAmmo, Capacity);
	return Added;
}
