#include "Game/FirstVRGameMode.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerHealth.h"

DEFINE_LOG_CATEGORY_STATIC(LogEncounter, Log, All);

void AFirstVRGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	// 폰이 스폰되고 컨트롤러에 빙의된 직후, 폰의 BeginPlay보다 먼저 불린다. 구독은 그 전에 해 둬도 된다.
	UPlayerHealth* Health = PlayerPawn ? PlayerPawn->FindComponentByClass<UPlayerHealth>() : nullptr;
	if (!Health)
	{
		UE_LOG(LogEncounter, Warning, TEXT("Player pawn %s has no PlayerHealth"), *GetNameSafe(PlayerPawn));
		return;
	}

	Health->OnDied.AddUniqueDynamic(this, &AFirstVRGameMode::HandlePlayerDied);
	UE_LOG(LogEncounter, Log, TEXT("Watching %s health"), *PlayerPawn->GetName());
}

void AFirstVRGameMode::HandlePlayerDied()
{
	SetEncounterState(EEncounterState::Failed);
}

void AFirstVRGameMode::SetEncounterState(EEncounterState NewState)
{
	if (EncounterState == NewState)
	{
		return;
	}

	EncounterState = NewState;
	UE_LOG(LogEncounter, Log, TEXT("Encounter %s"), *UEnum::GetDisplayValueAsText(NewState).ToString());
	OnEncounterStateChanged.Broadcast(NewState);
}

void AFirstVRGameMode::RestartEncounter()
{
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	UE_LOG(LogEncounter, Log, TEXT("Restarting %s"), *LevelName);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

static FAutoConsoleCommandWithWorld EncounterRestartCommand(
	TEXT("Encounter.Restart"),
	TEXT("Reloads the current level through the game mode."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (AFirstVRGameMode* GameMode = World ? World->GetAuthGameMode<AFirstVRGameMode>() : nullptr)
		{
			GameMode->RestartEncounter();
		}
	}));
