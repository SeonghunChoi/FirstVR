#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_EnemyStrikeWindow.generated.h"

/**
 * 공격 몽타주의 타격 판정 구간. 막대의 시작부터 끝까지 친 본이 지나간 자리를 반경 Radius로 판정한다.
 * 휘두르는 궤적 전체가 판정이라, 한 프레임 노티파이(Enemy Strike)보다 동작과 거리 차이에 덜 민감하다.
 * 판정과 피해는 메시 주인의 UEnemyAttack이 하고, 한 구간에서 한 번만 맞힌다.
 */
UCLASS(meta = (DisplayName = "Enemy Strike Window"))
class FIRSTVR_API UAnimNotifyState_EnemyStrikeWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** 치는 본. 이 본이 지나간 선분을 판정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strike")
	FName StrikeBone = TEXT("hand_r");

	/** 판정 반경. 본이 지나간 선분에서 플레이어 몸 캡슐 표면까지 이 거리 안이면 맞는다. 주먹이나 발의 크기를 포함한 값이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strike", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float Radius = 15.f;
};
