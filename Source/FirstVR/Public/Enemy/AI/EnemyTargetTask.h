#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "EnemyTargetTask.generated.h"

class APawn;

USTRUCT()
struct FIRSTVR_API FEnemyTargetTaskInstanceData
{
	GENERATED_BODY()

	/** 이 State Tree가 조종하는 적의 폰. State Tree AI 컴포넌트 스키마의 Actor 문맥에 자동으로 연결된다. */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<APawn> Actor = nullptr;

	/** 거리 안에 들어온 뒤 AttackRange보다 이만큼 더 멀어져야 거리 밖으로 본다. 경계에서 추적과 정지가 번갈아 바뀌지 않게 한다. */
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float RangeHysteresis = 50.f;

	/** 플레이어 머리(카메라) 아래 바닥 위치. Move To의 Destination에 바인딩한다. */
	UPROPERTY(EditAnywhere, Category = Output)
	FVector TargetLocation = FVector::ZeroVector;

	/** 적의 몸 중심에서 TargetLocation까지의 수평 거리. */
	UPROPERTY(EditAnywhere, Category = Output)
	float Distance = 0.f;

	/** 적 데이터의 공격 거리. Move To의 Acceptable Radius에 바인딩한다. */
	UPROPERTY(EditAnywhere, Category = Output)
	float AttackRange = 0.f;

	/** 공격 거리 안인지. 상태 선택과 전환 조건에 쓴다. */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bInRange = false;
};

/**
 * 적의 전역 태스크. 매 틱 플레이어 머리 아래 바닥 위치와 거리, 공격 거리 안인지를 계산해 출력한다.
 * 이동이나 공격은 하지 않는다. 다른 상태의 태스크와 조건이 이 출력에 바인딩한다.
 */
USTRUCT(meta = (DisplayName = "Enemy Target", Category = "Enemy"))
struct FIRSTVR_API FEnemyTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnemyTargetTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	void UpdateTarget(FInstanceDataType& Data) const;
};
