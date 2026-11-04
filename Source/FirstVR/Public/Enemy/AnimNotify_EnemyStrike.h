#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EnemyStrike.generated.h"

/**
 * 공격 몽타주의 타격 순간. 어느 본으로 몇 cm 반경을 치는지만 적어 두고, 판정과 피해는 메시 주인의 UEnemyAttack이 한다.
 * 한 몽타주에 여러 개를 놓으면 타격이 여러 번이다.
 */
UCLASS(meta = (DisplayName = "Enemy Strike"))
class FIRSTVR_API UAnimNotify_EnemyStrike : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** 치는 본. 이 본 위치를 판정 구의 중심으로 쓴다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strike")
	FName StrikeBone = TEXT("hand_r");

	/** 판정 반경. 본 위치에서 플레이어 몸 캡슐 표면까지 이 거리 안이면 맞는다. 주먹이나 발의 크기를 포함한 값이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strike", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float Radius = 15.f;
};
