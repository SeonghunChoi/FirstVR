#include "Game/EncounterSection.h"

#include "Components/ArrowComponent.h"
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

#if WITH_EDITORONLY_DATA
	FacingArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	if (FacingArrow)
	{
		FacingArrow->SetupAttachment(Root);
		FacingArrow->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
		FacingArrow->ArrowColor = FColor::Cyan;
		FacingArrow->ArrowSize = 1.5f;
	}
#endif
}

bool AEncounterSection::IsInsideStandArea(const FVector& WorldLocation) const
{
	// 표식이 둥근 원판이고 구간을 돌려도(도착 방향) 범위가 같도록 원으로 본다. 반지름 = 상자 X 반폭(월드 크기).
	const float Radius = ArrivalBox->GetScaledBoxExtent().X;
	return FVector::DistSquared2D(WorldLocation, ArrivalBox->GetComponentLocation()) <= FMath::Square(Radius);
}

void AEncounterSection::BeginPlay()
{
	Super::BeginPlay();

	Marker->SetVisibility(false);
	LockDoorsClosed();
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
	if (IsInsideStandArea(ViewLocation))
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
	OpenDoors();
}

void AEncounterSection::Arrive()
{
	GetWorldTimerManager().ClearTimer(ArrivalTimer);
	SetSectionState(ESectionState::Fighting);
	UE_LOG(LogSection, Log, TEXT("[%s] Player arrived at section %d"), *GetName(), SectionOrder);
	OnPlayerArrived.Broadcast(this);
}

void AEncounterSection::LockDoorsClosed() const
{
	for (AActor* Door : DoorsToUnlock)
	{
		if (!Door)
		{
			continue;
		}

		if (!SetDoorBool(Door, DoorLockPropertyName, true))
		{
			UE_LOG(LogSection, Warning, TEXT("[%s] %s has no bool '%s'"), *GetName(), *Door->GetName(), *DoorLockPropertyName.ToString());
			continue;
		}
		UE_LOG(LogSection, Log, TEXT("[%s] %s locked"), *GetName(), *Door->GetName());
	}
}

void AEncounterSection::OpenDoors() const
{
	for (AActor* Door : DoorsToUnlock)
	{
		if (!Door)
		{
			continue;
		}

		// 이미 열려 있으면 열기를 다시 시키지 않는다(팩 문은 열 때마다 닫힌 자리부터 다시 움직인다).
		bool bAlreadyOpen = false;
		if (GetDoorBool(Door, DoorOpenedPropertyName, bAlreadyOpen) && bAlreadyOpen)
		{
			UE_LOG(LogSection, Log, TEXT("[%s] %s already open"), *GetName(), *Door->GetName());
			continue;
		}

		// 열기만 하고 잠금(Locked)은 그대로 둔다. 잠긴 팩 문은 겹침을 무시하므로 다가가도 다시 열리지 않고, 멀어져도 닫히지 않는다.
		if (!SetDoorBool(Door, DoorOpenPropertyName, true))
		{
			UE_LOG(LogSection, Warning, TEXT("[%s] %s has no bool '%s'"), *GetName(), *Door->GetName(), *DoorOpenPropertyName.ToString());
			continue;
		}
		UE_LOG(LogSection, Log, TEXT("[%s] %s opening (kept open)"), *GetName(), *Door->GetName());
	}
}

bool AEncounterSection::GetDoorBool(const AActor* Door, FName PropertyName, bool& bOutValue)
{
	const FBoolProperty* Property = FindFProperty<FBoolProperty>(Door->GetClass(), PropertyName);
	if (!Property)
	{
		return false;
	}

	bOutValue = Property->GetPropertyValue_InContainer(Door);
	return true;
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
