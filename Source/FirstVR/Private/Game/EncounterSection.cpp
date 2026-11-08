#include "Game/EncounterSection.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Game/WaveDataAsset.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogSection, Log, All);

AEncounterSection::AEncounterSection()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// 사방 1.5m, 바닥에서 2m 높이까지(에디터에서 범위를 보이는 용도, 판정은 수평만). 충돌은 쓰지 않는다.
	ArrivalBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ArrivalBox"));
	ArrivalBox->SetupAttachment(Root);
	ArrivalBox->SetBoxExtent(FVector(75.f, 75.f, 100.f));
	ArrivalBox->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	ArrivalBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrivalBox->SetGenerateOverlapEvents(false);
	ArrivalBox->SetCanEverAffectNavigation(false);

	Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
	Marker->SetupAttachment(Root);
	Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Marker->SetCanEverAffectNavigation(false);
	Marker->SetVisibility(false);
}

void AEncounterSection::BeginPlay()
{
	Super::BeginPlay();

	Marker->SetVisibility(false);
	SetDoorsLocked(true);
	UE_LOG(LogSection, Log, TEXT("[%s] Section %d ready (%s, %d doors, heal %.0f)"), *GetName(), SectionOrder,
		HasWaves() ? *WaveData->GetName() : TEXT("no waves"), DoorsToUnlock.Num(), HealOnClear);
}

bool AEncounterSection::HasWaves() const
{
	return WaveData && !WaveData->Waves.IsEmpty();
}

void AEncounterSection::Activate()
{
	if (SectionState != ESectionState::Waiting)
	{
		return;
	}

	SetSectionState(ESectionState::Active);
	Marker->SetVisibility(true);

	// 도착할 때까지 짧은 간격으로 머리 위치를 본다. 시작할 때 폰이 아직 없거나 머리가 아직 자리를 잡기 전이어도 다음 확인에서 잡힌다.
	GetWorldTimerManager().SetTimer(ArrivalTimer, this, &AEncounterSection::CheckArrival, ArrivalCheckInterval, true, 0.f);
}

void AEncounterSection::CheckArrival()
{
	if (SectionState != ESectionState::Active)
	{
		GetWorldTimerManager().ClearTimer(ArrivalTimer);
		return;
	}

	const APlayerController* Player = GetWorld()->GetFirstPlayerController();
	if (!Player || !Player->GetPawn())
	{
		return;
	}

	// VR에서는 시점(카메라)이 머리다. 상자 기준 좌표로 바꿔 수평 범위만 본다(키·앉음과 상관없게).
	FVector ViewLocation;
	FRotator ViewRotation;
	Player->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector Local = ArrivalBox->GetComponentTransform().InverseTransformPosition(ViewLocation);
	const FVector Extent = ArrivalBox->GetUnscaledBoxExtent();
	if (FMath::Abs(Local.X) <= Extent.X && FMath::Abs(Local.Y) <= Extent.Y)
	{
		Arrive();
	}
}

void AEncounterSection::ForceArrive()
{
	if (SectionState == ESectionState::Active)
	{
		Arrive();
	}
}

void AEncounterSection::MarkCleared()
{
	if (SectionState == ESectionState::Cleared)
	{
		return;
	}

	SetSectionState(ESectionState::Cleared);
	Marker->SetVisibility(false);
	SetDoorsLocked(false);
}

void AEncounterSection::Arrive()
{
	GetWorldTimerManager().ClearTimer(ArrivalTimer);
	SetSectionState(ESectionState::Fighting);
	UE_LOG(LogSection, Log, TEXT("[%s] Player arrived at section %d"), *GetName(), SectionOrder);
	OnPlayerArrived.Broadcast(this);
}

void AEncounterSection::SetDoorsLocked(bool bLocked) const
{
	for (AActor* Door : DoorsToUnlock)
	{
		if (!Door)
		{
			continue;
		}

		if (!SetDoorBool(Door, DoorLockPropertyName, bLocked))
		{
			UE_LOG(LogSection, Warning, TEXT("[%s] %s has no bool '%s'"), *GetName(), *Door->GetName(), *DoorLockPropertyName.ToString());
			continue;
		}

		// 풀 때 바로 연다. 팩 문은 상자에 무언가 들어올 때만 열리므로, 플레이어가 멀리 있어도 끝났다는 것이 보이게 한다.
		const bool bOpened = !bLocked && !DoorOpenPropertyName.IsNone() && SetDoorBool(Door, DoorOpenPropertyName, true);
		UE_LOG(LogSection, Log, TEXT("[%s] %s %s"), *GetName(), *Door->GetName(), bLocked ? TEXT("locked") : (bOpened ? TEXT("unlocked, opening") : TEXT("unlocked")));
	}
}

bool AEncounterSection::SetDoorBool(AActor* Door, FName PropertyName, bool bValue)
{
	// 팩 블루프린트 변수라 C++ 타입을 모른다. 리플렉션으로 이름이 같은 bool 속성을 찾는다(유니티의 SerializedProperty와 비슷).
	FBoolProperty* Property = FindFProperty<FBoolProperty>(Door->GetClass(), PropertyName);
	if (!Property)
	{
		return false;
	}

	Property->SetPropertyValue_InContainer(Door, bValue);
	return true;
}

void AEncounterSection::SetSectionState(ESectionState NewState)
{
	if (SectionState == NewState)
	{
		return;
	}

	SectionState = NewState;
	OnSectionStateChanged(NewState);
}
