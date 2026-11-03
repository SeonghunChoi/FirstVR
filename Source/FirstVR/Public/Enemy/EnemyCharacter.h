#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UDamageReceiver;
class UHitReaction;
class UPhysicalAnimationComponent;

/**
 * 적의 몸. 캡슐 루트와 걷기 이동(ACharacter) 위에 피해 판정과 물리 피격 반응 컴포넌트를 만들어 둔다.
 * 피해, 절뚝임, 피격 반응, 래그돌 판단은 각 컴포넌트가 하고, 이 클래스는 몸 전체에 걸친 처리(사망 시 캡슐과 이동 정지)만 맡는다.
 * 메시와 위치, 적 데이터는 블루프린트 자식에서 정한다.
 */
UCLASS(Abstract)
class FIRSTVR_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	UDamageReceiver* GetDamageReceiver() const { return DamageReceiver; }

protected:
	virtual void BeginPlay() override;

	/** 피해 계산과 체력, 절뚝임, 사망 상태. 적 데이터(EnemyData)는 이 컴포넌트에 넣는다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UDamageReceiver> DamageReceiver;

	/** 물리로 도는 바디를 애니메이션 자세로 당기는 모터. UHitReaction이 쓴다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimation;

	/** 맞은 본 아래 물리 반응과 사망 래그돌. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UHitReaction> HitReaction;

private:
	UFUNCTION()
	void HandleDied();
};
