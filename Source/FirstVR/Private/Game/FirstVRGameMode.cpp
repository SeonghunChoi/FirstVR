#include "Game/FirstVRGameMode.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Enemy/AI/EnemyAIController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Game/WaveSpawner.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
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
	StartEncounter();
}

void AFirstVRGameMode::StartEncounter()
{
	WaveSpawner->StartWaves();
}

void AFirstVRGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	// 지금 폰은 PlayerStart 자리에 있다. 이 자리를 기억해 두고, 카메라가 헤드셋 자세를 받은 뒤 머리를 여기에 맞춘다.
	if (PlayerPawn && bAlignHeadToPlayerStart)
	{
		FTimerHandle AlignTimer;
		GetWorldTimerManager().SetTimer(AlignTimer,
			FTimerDelegate::CreateUObject(this, &AFirstVRGameMode::AlignHeadToStart, TWeakObjectPtr<APawn>(PlayerPawn), PlayerPawn->GetActorTransform()),
			FMath::Max(HeadAlignDelay, 0.01f), false);
	}

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

void AFirstVRGameMode::AlignHeadToStart(TWeakObjectPtr<APawn> PlayerPawn, FTransform Start)
{
	APawn* Pawn = PlayerPawn.Get();
	const UCameraComponent* Camera = Pawn ? Pawn->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!Camera)
	{
		return;
	}

	// 폰 기준 카메라 자세 = 트래킹 공간에서 머리가 원점에서 떨어진 거리·돈 각도(로그용).
	const FTransform CameraInPawn = Camera->GetComponentTransform().GetRelativeTransform(Pawn->GetActorTransform());
	const FVector HeadOffset(CameraInPawn.GetLocation().X, CameraInPawn.GetLocation().Y, 0.f);

	MoveHeadTo(Pawn, Start.GetLocation(), Start.Rotator().Yaw);
	UE_LOG(LogEncounter, Log, TEXT("Head aligned to start (head offset %.0f cm, yaw %.0f)"), HeadOffset.Size(), CameraInPawn.Rotator().Yaw);
}

void AFirstVRGameMode::MoveHeadTo(APawn* PlayerPawn, const FVector& Location, float FacingYaw)
{
	const UCameraComponent* Camera = PlayerPawn ? PlayerPawn->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!Camera)
	{
		return;
	}

	// 폰 기준 카메라 자세 = 트래킹 공간에서 머리가 원점에서 떨어진 거리·돈 각도.
	const FTransform CameraInPawn = Camera->GetComponentTransform().GetRelativeTransform(PlayerPawn->GetActorTransform());
	const FVector HeadOffset(CameraInPawn.GetLocation().X, CameraInPawn.GetLocation().Y, 0.f);

	// 폰을 (원하는 방향 - 머리 각도)로 돌리고, 돌린 머리 거리만큼 반대로 옮기면 머리가 그 자리에서 그 방향을 본다.
	const FRotator NewRotation(0.f, FacingYaw - CameraInPawn.Rotator().Yaw, 0.f);
	const FVector NewLocation = Location - NewRotation.RotateVector(HeadOffset);
	PlayerPawn->SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
	if (AController* Controller = PlayerPawn->GetController())
	{
		Controller->SetControlRotation(NewRotation);   // 컨트롤러 방향을 따르는 폰이면 다음 틱에 되돌아가지 않게
	}
}

float AFirstVRGameMode::GetHeadYaw(const APawn* PlayerPawn)
{
	const UCameraComponent* Camera = PlayerPawn ? PlayerPawn->FindComponentByClass<UCameraComponent>() : nullptr;
	return Camera ? Camera->GetComponentRotation().Yaw : (PlayerPawn ? PlayerPawn->GetActorRotation().Yaw : 0.f);
}

bool AFirstVRGameMode::ResolveTeleportDestination(const FVector& AimedLocation, FVector& OutDestination, TOptional<float>& OutFacingYaw) const
{
	OutDestination = AimedLocation;
	OutFacingYaw.Reset();
	return true;
}

bool AFirstVRGameMode::CanTeleportTo(FVector AimedLocation, FVector& Destination)
{
	TOptional<float> FacingYaw;
	return ResolveTeleportDestination(AimedLocation, Destination, FacingYaw);
}

float AFirstVRGameMode::GetTeleportPreviewYaw(const UObject* WorldContextObject, APawn* PlayerPawn, FVector AimedLocation)
{
	const float PawnYaw = PlayerPawn ? PlayerPawn->GetActorRotation().Yaw : 0.f;
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AFirstVRGameMode* GameMode = World ? World->GetAuthGameMode<AFirstVRGameMode>() : nullptr;
	const UCameraComponent* Camera = PlayerPawn ? PlayerPawn->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!GameMode || !Camera)
	{
		return PawnYaw;
	}

	FVector Destination;
	TOptional<float> FacingYaw;
	if (!GameMode->ResolveTeleportDestination(AimedLocation, Destination, FacingYaw) || !FacingYaw.IsSet())
	{
		return PawnYaw;
	}

	// MoveHeadTo와 같은 계산: 폰 Yaw = 정한 방향 - 머리가 폰 안에서 돈 각도
	const float HeadYawInPawn = Camera->GetComponentTransform().GetRelativeTransform(PlayerPawn->GetActorTransform()).Rotator().Yaw;
	return FacingYaw.GetValue() - HeadYawInPawn;
}

bool AFirstVRGameMode::TeleportPlayerTo(APawn* PlayerPawn, FVector AimedLocation)
{
	FVector Destination;
	TOptional<float> FacingYaw;
	if (!PlayerPawn || !ResolveTeleportDestination(AimedLocation, Destination, FacingYaw))
	{
		return false;
	}

	const float Yaw = FacingYaw.Get(GetHeadYaw(PlayerPawn));
	MoveHeadTo(PlayerPawn, Destination, Yaw);
	UE_LOG(LogEncounter, Log, TEXT("Teleported to %s (yaw %.0f%s)"), *Destination.ToCompactString(), Yaw, FacingYaw.IsSet() ? TEXT(", fixed") : TEXT(""));
	return true;
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
	SpawnResultScreen(FailScreenClass);
}

void AFirstVRGameMode::HandleAllWavesCleared()
{
	FinishWithVictory();
}

void AFirstVRGameMode::FinishWithVictory()
{
	if (EncounterState != EEncounterState::Playing)
	{
		return;
	}

	SetEncounterState(EEncounterState::Victory);
	SpawnResultScreen(VictoryScreenClass);
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

void AFirstVRGameMode::SpawnResultScreen(TSubclassOf<AActor> ScreenClass)
{
	if (!ScreenClass || ResultScreen)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return;
	}

	// 머리 앞, 수평 방향으로 ResultScreenDistance만큼. 화면 앞면이 머리를 보도록 돌려 둔다.
	const FVector Head = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator Facing(0.f, PlayerController->PlayerCameraManager->GetCameraRotation().Yaw, 0.f);
	const FVector Location = Head + Facing.Vector() * ResultScreenDistance;
	const FRotator Rotation(0.f, Facing.Yaw + 180.f, 0.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ResultScreen = GetWorld()->SpawnActor<AActor>(ScreenClass, Location, Rotation, Params);
	UE_LOG(LogEncounter, Log, TEXT("Result screen %s spawned"), *GetNameSafe(ResultScreen));
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

void AFirstVRGameMode::OpenNextLevel()
{
	if (NextLevel.IsNull())
	{
		UE_LOG(LogEncounter, Warning, TEXT("No next level"));
		return;
	}

	// 소프트 참조의 패키지 이름(/Game/.../L_Lab)으로 연다. 쿠킹할 맵 목록에 그 레벨이 있어야 패키지에서 열린다.
	const FString LevelName = NextLevel.ToSoftObjectPath().GetLongPackageName();
	UE_LOG(LogEncounter, Log, TEXT("Opening next level %s"), *LevelName);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

static FAutoConsoleCommandWithWorld EncounterNextLevelCommand(
	TEXT("Encounter.NextLevel"),
	TEXT("Opens the game mode's next level."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (AFirstVRGameMode* GameMode = World ? World->GetAuthGameMode<AFirstVRGameMode>() : nullptr)
		{
			GameMode->OpenNextLevel();
		}
	}));

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
