#include "Weapon/MagazineDispenser.h"

#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Weapon/Magazine.h"
#include "Weapon/WeaponTypes.h"

namespace
{
	/** 가져갔는지 보는 간격(초). 사람이 집는 동작을 놓치지 않을 만큼이면 된다. */
	constexpr float DispenserCheckInterval = 0.25f;
}

AMagazineDispenser::AMagazineDispenser()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

#if WITH_EDITORONLY_DATA
	Arrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (Arrow)
	{
		Arrow->SetupAttachment(Root);
		Arrow->ArrowColor = FColor::Yellow;
		Arrow->ArrowSize = 0.5f;
		Arrow->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	}
#endif
}

void AMagazineDispenser::BeginPlay()
{
	Super::BeginPlay();

	if (!MagazineClass)
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] MagazineClass is not set"), *GetName());
		return;
	}

	Dispense();
	// 틱 대신 타이머로 띄엄띄엄 본다. 유니티로 치면 InvokeRepeating.
	GetWorldTimerManager().SetTimer(CheckTimer, this, &AMagazineDispenser::CheckTaken, DispenserCheckInterval, true);
}

void AMagazineDispenser::Dispense()
{
	const FVector Location = GetActorLocation() + FVector(0.f, 0.f, SpawnHeight);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CurrentMagazine = GetWorld()->SpawnActor<AMagazine>(MagazineClass, Location, GetActorRotation(), Params);

	if (CurrentMagazine)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Dispensed %s"), *GetName(), *CurrentMagazine->GetName());
		OnMagazineDispensed(CurrentMagazine);
	}
}

void AMagazineDispenser::CheckTaken()
{
	if (GetWorldTimerManager().IsTimerActive(RespawnTimer))
	{
		return;
	}

	const AMagazine* Magazine = CurrentMagazine;
	const bool bTaken = !IsValid(Magazine)
		|| Magazine->IsInserted()
		|| Magazine->GetAttachParentActor() != nullptr
		|| FVector::Dist(Magazine->GetActorLocation(), GetActorLocation()) > TakeDistance + SpawnHeight;
	if (!bTaken)
	{
		return;
	}

	CurrentMagazine = nullptr;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Magazine taken, next in %.1fs"), *GetName(), RespawnDelay);
	GetWorldTimerManager().SetTimer(RespawnTimer, this, &AMagazineDispenser::Dispense, FMath::Max(RespawnDelay, 0.01f), false);
}
