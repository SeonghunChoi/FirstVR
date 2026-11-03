#include "Enemy/AI/EnemyAIController.h"

#include "Components/StateTreeAIComponent.h"
#include "Enemy/DamageReceiver.h"

AEnemyAIController::AEnemyAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	// 레벨에 배치한 적은 게임 시작 전에 폰을 잡는다. 컴포넌트가 스스로 시작하지 않게 하고 TryStartLogic에서 시작한다.
	StateTreeAI->SetStartLogicAutomatically(false);
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	TryStartLogic();
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	TryStartLogic();
}

void AEnemyAIController::OnUnPossess()
{
	if (StateTreeAI->IsRunning())
	{
		StateTreeAI->StopLogic(TEXT("Unpossessed"));
		UE_LOG(LogEnemy, Log, TEXT("[%s] State Tree stopped"), *GetName());
	}
	Super::OnUnPossess();
}

void AEnemyAIController::TryStartLogic()
{
	if (!HasActorBegunPlay() || !GetPawn() || StateTreeAI->IsRunning())
	{
		return;
	}

	StateTreeAI->StartLogic();
	UE_LOG(LogEnemy, Log, TEXT("[%s] State Tree started on %s"), *GetName(), *GetNameSafe(GetPawn()));
}
