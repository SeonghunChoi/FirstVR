#include "Game/FirstVRGameMode.h"

#include "Camera/PlayerCameraManager.h"
#include "Enemy/AI/EnemyAIController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Game/WaveSpawner.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerHealth.h"

DEFINE_LOG_CATEGORY_STATIC(LogEncounter, Log, All);

AFirstVRGameMode::AFirstVRGameMode()
{
	WaveSpawner = CreateDefaultSubobject<UWaveSpawner>(TEXT("WaveSpawner"));
}

void AFirstVRGameMode::BeginPlay()
{
	Super::BeginPlay();

	WaveSpawner->OnAllWavesCleared.AddUniqueDynamic(this, &AFirstVRGameMode::HandleAllWavesCleared);
	WaveSpawner->StartWaves();
}

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
	// 이미 승리했으면 그 뒤의 사망은 결과를 바꾸지 않는다.
	if (EncounterState != EEncounterState::Playing)
	{
		return;
	}

	SetEncounterState(EEncounterState::Failed);
	WaveSpawner->StopWaves();
	HaltEnemies(TEXT("Encounter failed"));
	SpawnFailScreen();
}

void AFirstVRGameMode::HandleAllWavesCleared()
{
	if (EncounterState != EEncounterState::Playing)
	{
		return;
	}

	SetEncounterState(EEncounterState::Victory);
}

void AFirstVRGameMode::HaltEnemies(const FString& Reason) const
{
	int32 Count = 0;
	for (TActorIterator<AEnemyAIController> It(GetWorld()); It; ++It)
	{
		It->HaltLogic(Reason);
		++Count;
	}
	UE_LOG(LogEncounter, Log, TEXT("Halted %d enemies"), Count);
}

void AFirstVRGameMode::SpawnFailScreen()
{
	if (!FailScreenClass || FailScreen)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return;
	}

	// 머리 앞, 수평 방향으로 FailScreenDistance만큼. 화면 앞면이 머리를 보도록 돌려 둔다.
	const FVector Head = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator Facing(0.f, PlayerController->PlayerCameraManager->GetCameraRotation().Yaw, 0.f);
	const FVector Location = Head + Facing.Vector() * FailScreenDistance;
	const FRotator Rotation(0.f, Facing.Yaw + 180.f, 0.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FailScreen = GetWorld()->SpawnActor<AActor>(FailScreenClass, Location, Rotation, Params);
	UE_LOG(LogEncounter, Log, TEXT("Fail screen %s spawned"), *GetNameSafe(FailScreen));
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
