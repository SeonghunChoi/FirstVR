#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "PlayerBody.generated.h"

class UCameraComponent;

/**
 * 플레이어(VR 폰)의 몸. 바닥에서 머리 위까지 세운 캡슐이 매 틱 머리(HMD)를 따라간다.
 * 적 캡슐을 막아서 공격 루트 모션이 플레이어를 통과하지 못하게 하고, 적 공격(UEnemyAttack)의 판정 대상이 된다.
 * 폰의 루트 아래에 붙인다. 폰 위치를 바닥(트래킹 원점)으로 본다.
 */
UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UPlayerBody : public UCapsuleComponent
{
	GENERATED_BODY()

public:
	UPlayerBody();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 점에서 캡슐 표면까지의 거리. 캡슐 안이면 0 이하. */
	float GetDistanceToSurface(const FVector& Point) const;

	/** 선분(Start → End)에서 캡슐 표면까지의 가장 가까운 거리. 선분이 캡슐을 지나가면 0 이하. */
	float GetDistanceToSurface(const FVector& Start, const FVector& End) const;

protected:
	virtual void BeginPlay() override;

	/** 몸 반경. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "1.0", ForceUnits = "cm"))
	float BodyRadius = 25.f;

	/** 눈(HMD) 위로 캡슐을 더 올리는 높이. 정수리까지다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float HeadTopMargin = 10.f;

private:
	/** 캡슐을 머리 아래 바닥부터 머리 위까지로 맞춘다. */
	void FollowHead();

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> Camera;
};
