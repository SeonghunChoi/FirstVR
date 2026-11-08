#include "Game/FirstVRSectionGameMode.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Game/EncounterSection.h"
#include "Game/WaveDataAsset.h"
#include "Game/WaveSpawner.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Player/PlayerHealth.h"

DEFINE_LOG_CATEGORY_STATIC(LogEncounterSection, Log, All);

void AFirstVRSectionGameMode::StartEncounter()
{
	Sections.Reset();
	for (TActorIterator<AEncounterSection> It(GetWorld()); It; ++It)
	{
		Sections.Add(*It);
	}
	Sections.Sort([](const AEncounterSection& A, const AEncounterSection& B) { return A.GetSectionOrder() < B.GetSectionOrder(); });

	if (Sections.IsEmpty())
	{
		UE_LOG(LogEncounterSection, Warning, TEXT("No EncounterSection in level"));
		return;
	}

	for (AEncounterSection* Section : Sections)
	{
		Section->OnPlayerArrived.AddUniqueDynamic(this, &AFirstVRSectionGameMode::HandleSectionArrived);
	}
	UE_LOG(LogEncounterSection, Log, TEXT("%d sections"), Sections.Num());
	ActivateSection(0);
}

void AFirstVRSectionGameMode::ActivateSection(int32 Index)
{
	CurrentSectionIndex = Index;
	AEncounterSection* Section = Sections[Index];
	UE_LOG(LogEncounterSection, Log, TEXT("Section %d/%d active (%s)"), Index + 1, Sections.Num(), *Section->GetName());
	Section->Activate();   // 플레이어가 이미 설 자리에 있으면 이 안에서 바로 도착이 온다
}

void AFirstVRSectionGameMode::HandleSectionArrived(AEncounterSection* Section)
{
	if (GetEncounterState() != EEncounterState::Playing || !Sections.IsValidIndex(CurrentSectionIndex) || Sections[CurrentSectionIndex] != Section)
	{
		return;
	}

	if (!Section->HasWaves())
	{
		UE_LOG(LogEncounterSection, Log, TEXT("Section %d has no waves"), CurrentSectionIndex + 1);
		CompleteCurrentSection();
		return;
	}

	UE_LOG(LogEncounterSection, Log, TEXT("Section %d fight (%s)"), CurrentSectionIndex + 1, *Section->GetWaveData()->GetName());
	WaveSpawner->SetWaveData(Section->GetWaveData());
	WaveSpawner->StartWaves();
}

void AFirstVRSectionGameMode::HandleAllWavesCleared()
{
	// 부모는 여기서 승리지만, 구간 레벨에서는 지금 구간의 웨이브가 끝난 것뿐이다.
	if (GetEncounterState() == EEncounterState::Playing && Sections.IsValidIndex(CurrentSectionIndex)
		&& Sections[CurrentSectionIndex]->GetSectionState() == ESectionState::Fighting)
	{
		CompleteCurrentSection();
	}
}

void AFirstVRSectionGameMode::CompleteCurrentSection()
{
	AEncounterSection* Section = Sections[CurrentSectionIndex];
	UE_LOG(LogEncounterSection, Log, TEXT("Section %d/%d cleared"), CurrentSectionIndex + 1, Sections.Num());
	HealPlayer(Section->GetHealOnClear());
	Section->MarkCleared();

	if (Sections.IsValidIndex(CurrentSectionIndex + 1))
	{
		ActivateSection(CurrentSectionIndex + 1);
		return;
	}

	CurrentSectionIndex = INDEX_NONE;
	UE_LOG(LogEncounterSection, Log, TEXT("All sections cleared"));
	FinishWithVictory();
}

bool AFirstVRSectionGameMode::ResolveTeleportDestination(const FVector& AimedLocation, FVector& OutDestination, TOptional<float>& OutFacingYaw) const
{
	OutFacingYaw.Reset();
	const AEncounterSection* Section = Sections.IsValidIndex(CurrentSectionIndex) ? Sections[CurrentSectionIndex].Get() : nullptr;
	if (GetEncounterState() != EEncounterState::Playing || !Section || Section->GetSectionState() != ESectionState::Active
		|| !Section->IsInsideStandArea(AimedLocation))
	{
		return false;
	}

	OutDestination = Section->GetActorLocation();
	OutFacingYaw = Section->GetActorRotation().Yaw;
	return true;
}

void AFirstVRSectionGameMode::HealPlayer(float Amount) const
{
	if (Amount <= 0.f)
	{
		return;
	}

	const APlayerController* Player = GetWorld()->GetFirstPlayerController();
	const APawn* Pawn = Player ? Player->GetPawn() : nullptr;
	if (UPlayerHealth* Health = Pawn ? Pawn->FindComponentByClass<UPlayerHealth>() : nullptr)
	{
		Health->Heal(Amount);
	}
}

void AFirstVRSectionGameMode::ForceArriveAtCurrentSection()
{
	if (Sections.IsValidIndex(CurrentSectionIndex))
	{
		Sections[CurrentSectionIndex]->ForceArrive();
	}
}

static FAutoConsoleCommandWithWorld EncounterArriveNextCommand(
	TEXT("Encounter.ArriveNext"),
	TEXT("Treats the player as arrived at the current section's spot (section levels only)."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (AFirstVRSectionGameMode* GameMode = World ? World->GetAuthGameMode<AFirstVRSectionGameMode>() : nullptr)
		{
			GameMode->ForceArriveAtCurrentSection();
		}
	}));
