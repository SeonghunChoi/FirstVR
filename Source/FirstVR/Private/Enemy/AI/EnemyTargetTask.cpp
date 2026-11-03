#include "Enemy/AI/EnemyTargetTask.h"

#include "Camera/PlayerCameraManager.h"
#include "Enemy/DamageReceiver.h"
#include "Enemy/EnemyDataAsset.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FEnemyTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.bInRange = false;
	UpdateTarget(Data);

	UE_LOG(LogEnemy, Log, TEXT("[%s] Target tracking started (distance %.0f, range %.0f)"), *GetNameSafe(Data.Actor), Data.Distance, Data.AttackRange);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FEnemyTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	UpdateTarget(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FEnemyTargetTask::UpdateTarget(FInstanceDataType& Data) const
{
	if (!Data.Actor)
	{
		return;
	}

	const UDamageReceiver* Receiver = Data.Actor->FindComponentByClass<UDamageReceiver>();
	const UEnemyDataAsset* EnemyData = Receiver ? Receiver->GetEnemyData() : nullptr;
	Data.AttackRange = EnemyData ? EnemyData->AttackRange : 0.f;

	// VR에서 폰의 위치는 트래킹 원점(플레이 공간 바닥)이다. 플레이어 본인은 카메라(HMD) 위치에 있다.
	const APlayerController* Player = UGameplayStatics::GetPlayerController(Data.Actor, 0);
	const APlayerCameraManager* Camera = Player ? Player->PlayerCameraManager.Get() : nullptr;
	if (!Camera)
	{
		return;
	}

	const FVector Head = Camera->GetCameraLocation();
	const APawn* PlayerPawn = Player->GetPawn();
	const float FloorZ = PlayerPawn ? PlayerPawn->GetActorLocation().Z : Head.Z;
	Data.TargetLocation = FVector(Head.X, Head.Y, FloorZ);
	Data.Distance = FVector::Dist2D(Data.Actor->GetActorLocation(), Data.TargetLocation);

	// Move To는 AttackRange에 닿으면 멈추지만 몇 cm 바깥에서 멈출 수 있다. 그 자리를 거리 안으로 쳐야 추적이 다시 시작되지 않는다.
	constexpr float ReachTolerance = 10.f;
	const bool bWasInRange = Data.bInRange;
	const float Limit = Data.AttackRange + (bWasInRange ? Data.RangeHysteresis : ReachTolerance);
	Data.bInRange = Data.Distance <= Limit;

	if (Data.bInRange != bWasInRange)
	{
		UE_LOG(LogEnemy, Log, TEXT("[%s] Target %s (distance %.0f, range %.0f)"), *Data.Actor->GetName(), Data.bInRange ? TEXT("in range") : TEXT("out of range"), Data.Distance, Data.AttackRange);
	}
}
