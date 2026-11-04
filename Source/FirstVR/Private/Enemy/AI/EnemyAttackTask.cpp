#include "Enemy/AI/EnemyAttackTask.h"

#include "Enemy/EnemyAttack.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

namespace
{
	UEnemyAttack* FindAttack(const FEnemyAttackInstanceData& Data)
	{
		return Data.Actor ? Data.Actor->FindComponentByClass<UEnemyAttack>() : nullptr;
	}
}

EStateTreeRunStatus FEnemyAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	UEnemyAttack* Attack = FindAttack(Context.GetInstanceData(*this));
	return Attack && Attack->StartAttack() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FEnemyAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const UEnemyAttack* Attack = FindAttack(Context.GetInstanceData(*this));
	return Attack && Attack->IsAttacking() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

void FEnemyAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (UEnemyAttack* Attack = FindAttack(Context.GetInstanceData(*this)); Attack && Attack->IsAttacking())
	{
		Attack->CancelAttack();
	}
}

bool FEnemyCanAttackCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const UEnemyAttack* Attack = FindAttack(Context.GetInstanceData(*this));
	return Attack && Attack->CanAttack();
}
