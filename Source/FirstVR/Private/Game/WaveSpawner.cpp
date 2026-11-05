#include "Game/WaveSpawner.h"

#include "Components/CapsuleComponent.h"
#include "Enemy/DamageReceiver.h"
#include "Enemy/EnemyCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Game/EnemySpawnPoint.h"
#include "Game/WaveDataAsset.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogWave, Log, All);

UWaveSpawner::UWaveSpawner()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UWaveSpawner::GetWaveCount() const
{
	return WaveData ? WaveData->Waves.Num() : 0;
}

void UWaveSpawner::StartWaves()
{
	if (!WaveData || WaveData->Waves.IsEmpty())
	{
		UE_LOG(LogWave, Warning, TEXT("[%s] No WaveData or no waves"), *GetOwner()->GetName());
		return;
	}

	bRunning = true;
	CurrentWaveIndex = INDEX_NONE;
	UE_LOG(LogWave, Log, TEXT("%d waves from %s, first in %.1fs"), WaveData->Waves.Num(), *WaveData->GetName(), WaveData->StartDelay);
	ScheduleWave(0, WaveData->StartDelay);
}

void UWaveSpawner::StopWaves()
{
	if (!bRunning)
	{
		return;
	}

	bRunning = false;
	bWaveActive = false;
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	UE_LOG(LogWave, Log, TEXT("Waves stopped at wave %d/%d"), GetCurrentWaveNumber(), GetWaveCount());
}

void UWaveSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void UWaveSpawner::ScheduleWave(int32 WaveIndex, float Delay)
{
	if (Delay <= 0.f)
	{
		StartWave(WaveIndex);
		return;
	}

	// 유니티의 Invoke(nameof(...), delay)와 같다. 람다 대신 payload를 넘기는 UObject 델리게이트를 쓴다.
	const FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UWaveSpawner::StartWave, WaveIndex);
	GetWorld()->GetTimerManager().SetTimer(WaveTimer, Delegate, Delay, false);
}

void UWaveSpawner::StartWave(int32 WaveIndex)
{
	if (!bRunning || !WaveData->Waves.IsValidIndex(WaveIndex))
	{
		return;
	}

	CurrentWaveIndex = WaveIndex;
	bWaveActive = true;

	const FWaveDefinition& Wave = WaveData->Waves[WaveIndex];
	GroupProgress.Reset();
	GroupProgress.SetNum(Wave.Groups.Num());

	int32 EnemyCount = 0;
	FTimerManager& Timers = GetWorld()->GetTimerManager();
	for (int32 GroupIndex = 0; GroupIndex < Wave.Groups.Num(); ++GroupIndex)
	{
		const FWaveSpawnGroup& Group = Wave.Groups[GroupIndex];
		if (!Group.EnemyClass)
		{
			UE_LOG(LogWave, Warning, TEXT("Wave %d group %d has no EnemyClass"), WaveIndex + 1, GroupIndex);
			continue;
		}

		GroupProgress[GroupIndex].Remaining = Group.Count;
		EnemyCount += Group.Count;

		// 첫 적은 StartDelay 뒤, 그다음부터는 SpawnInterval마다. 간격이 0이면 첫 호출에서 한꺼번에 내보내므로 반복 주기는 아무 값이나 된다.
		const FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UWaveSpawner::SpawnFromGroup, GroupIndex);
		const float Interval = Group.SpawnInterval > 0.f ? Group.SpawnInterval : 1.f;
		Timers.SetTimer(GroupProgress[GroupIndex].Timer, Delegate, Interval, true, FMath::Max(Group.StartDelay, 0.01f));
	}

	UE_LOG(LogWave, Log, TEXT("Wave %d/%d started (%d enemies)"), GetCurrentWaveNumber(), GetWaveCount(), EnemyCount);
	OnWaveStarted.Broadcast(GetCurrentWaveNumber(), GetWaveCount());

	// 묶음이 모두 비어 있으면 바로 끝난다.
	CheckWaveCleared();
}

void UWaveSpawner::SpawnFromGroup(int32 GroupIndex)
{
	if (!bWaveActive || !GroupProgress.IsValidIndex(GroupIndex))
	{
		return;
	}

	FGroupProgress& Progress = GroupProgress[GroupIndex];
	const FWaveSpawnGroup& Group = WaveData->Waves[CurrentWaveIndex].Groups[GroupIndex];
	const int32 SpawnCount = Group.SpawnInterval > 0.f ? 1 : Progress.Remaining;

	for (int32 i = 0; i < SpawnCount && Progress.Remaining > 0; ++i)
	{
		// 스폰에 실패해도 남은 수는 줄인다. 그러지 않으면 웨이브가 끝나지 않는다.
		--Progress.Remaining;
		if (AEnemyCharacter* Enemy = SpawnEnemy(Group))
		{
			AliveEnemies.Add(Enemy);
			Enemy->GetDamageReceiver()->OnDied.AddUniqueDynamic(this, &UWaveSpawner::HandleEnemyDied);
		}
	}

	if (Progress.Remaining <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(Progress.Timer);
		CheckWaveCleared();
	}
}

AEnemyCharacter* UWaveSpawner::SpawnEnemy(const FWaveSpawnGroup& Group) const
{
	const AEnemySpawnPoint* Point = FindSpawnPoint(Group.SpawnPointName);
	if (!Point)
	{
		UE_LOG(LogWave, Warning, TEXT("Spawn point '%s' not found"), *Group.SpawnPointName.ToString());
		return nullptr;
	}

	// 스폰 지점은 바닥이고 캐릭터 액터 위치는 캡슐 중심이다. 클래스 기본 객체(CDO)의 캡슐 반 높이만큼 올린다.
	const AEnemyCharacter* Defaults = Group.EnemyClass->GetDefaultObject<AEnemyCharacter>();
	const float HalfHeight = Defaults->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Location = Point->GetActorLocation() + FVector(0.f, 0.f, HalfHeight);
	const FRotator Rotation(0.f, Point->GetActorRotation().Yaw, 0.f);

	FActorSpawnParameters Params;
	// 같은 자리에 먼저 나온 적이 서 있으면 옆으로 비켜서 스폰한다.
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AEnemyCharacter* Enemy = GetWorld()->SpawnActor<AEnemyCharacter>(Group.EnemyClass, Location, Rotation, Params);

	UE_LOG(LogWave, Log, TEXT("Spawned %s at %s"), *GetNameSafe(Enemy), *Group.SpawnPointName.ToString());
	return Enemy;
}

AEnemySpawnPoint* UWaveSpawner::FindSpawnPoint(FName SpawnPointName) const
{
	for (TActorIterator<AEnemySpawnPoint> It(GetWorld()); It; ++It)
	{
		if (It->GetSpawnPointName() == SpawnPointName)
		{
			return *It;
		}
	}
	return nullptr;
}

void UWaveSpawner::HandleEnemyDied()
{
	// OnDied에는 누가 죽었는지가 없어서, 죽었거나 사라진 적을 한꺼번에 걸러 낸다.
	AliveEnemies.RemoveAll([](const AEnemyCharacter* Enemy)
	{
		return !IsValid(Enemy) || Enemy->GetDamageReceiver()->IsDead();
	});

	UE_LOG(LogWave, Log, TEXT("Enemy down, %d alive"), AliveEnemies.Num());
	CheckWaveCleared();
}

void UWaveSpawner::CheckWaveCleared()
{
	if (!bWaveActive)
	{
		return;
	}

	for (const FGroupProgress& Progress : GroupProgress)
	{
		if (Progress.Remaining > 0)
		{
			return;
		}
	}

	if (!AliveEnemies.IsEmpty())
	{
		return;
	}

	bWaveActive = false;
	UE_LOG(LogWave, Log, TEXT("Wave %d/%d cleared"), GetCurrentWaveNumber(), GetWaveCount());
	OnWaveCleared.Broadcast(GetCurrentWaveNumber());

	const int32 NextIndex = CurrentWaveIndex + 1;
	if (!WaveData->Waves.IsValidIndex(NextIndex))
	{
		bRunning = false;
		UE_LOG(LogWave, Log, TEXT("All waves cleared"));
		OnAllWavesCleared.Broadcast();
		return;
	}

	const float Rest = WaveData->Waves[CurrentWaveIndex].RestAfterClear;
	UE_LOG(LogWave, Log, TEXT("Next wave in %.1fs"), Rest);
	ScheduleWave(NextIndex, Rest);
}
