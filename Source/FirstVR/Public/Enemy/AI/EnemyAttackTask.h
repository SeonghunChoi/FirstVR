#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "StateTreeTaskBase.h"
#include "EnemyAttackTask.generated.h"

class APawn;

USTRUCT()
struct FIRSTVR_API FEnemyAttackInstanceData
{
	GENERATED_BODY()

	/** 이 State Tree가 조종하는 적의 폰. State Tree AI 컴포넌트 스키마의 Actor 문맥에 자동으로 연결된다. */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APawn> Actor = nullptr;
};

/**
 * 적의 UEnemyAttack에 공격을 요청하고, 공격이 끝날 때까지 Running이다. 끝나면 Succeeded, 시작하지 못하면 Failed.
 * 상태가 중간에 바뀌면 공격을 멈춘다. 공격 내용(회전, 몽타주, 판정)은 컴포넌트가 한다.
 */
USTRUCT(meta = (DisplayName = "Enemy Attack", Category = "Enemy"))
struct FIRSTVR_API FEnemyAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnemyAttackInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

/** 적의 UEnemyAttack이 지금 공격을 시작할 수 있는지(공격 중이 아니고, 쿨다운이 지났고, 살아 있는지). */
USTRUCT(meta = (DisplayName = "Enemy Can Attack", Category = "Enemy"))
struct FIRSTVR_API FEnemyCanAttackCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnemyAttackInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
