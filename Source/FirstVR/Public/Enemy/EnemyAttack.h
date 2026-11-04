#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAttack.generated.h"

class APawn;
class UAnimMontage;
class UDamageReceiver;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyStrike, bool, bHit, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyAttackFinished);

/**
 * 적의 근접 공격. 공격 요청을 받으면 플레이어 머리 쪽으로 돈 뒤 적 데이터의 공격 몽타주 하나를 재생한다.
 * 타격 구간은 몽타주의 Enemy Strike Window 노티파이 스테이트(한 프레임이면 Enemy Strike 노티파이)가 알린다. 친 본이 지나간 자리의 구가 플레이어 몸(UPlayerBody 캡슐)에 닿으면 ApplyDamage로 피해를 보낸다. 몸이 없으면 머리(HMD) 위치로 판정한다.
 * 언제 공격할지는 모른다. State Tree 같은 판단 쪽이 CanAttack을 보고 StartAttack을 부른다.
 */
UCLASS(ClassGroup = (Enemy), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UEnemyAttack : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAttack();

	/** 공격을 시작한다. 공격할 수 없으면(공격 중, 쿨다운, 사망) false. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
	bool StartAttack();

	/** 진행 중인 공격을 멈춘다. 몽타주도 멈추고 쿨다운을 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
	void CancelAttack();

	/** 지금 StartAttack을 부르면 시작되는지. */
	UFUNCTION(BlueprintPure, Category = "Enemy|Attack")
	bool CanAttack() const;

	/** 머리 쪽으로 도는 중이거나 몽타주를 재생하는 중인지. */
	UFUNCTION(BlueprintPure, Category = "Enemy|Attack")
	bool IsAttacking() const { return bAttacking; }

	/**
	 * 타격 판정 구간 시작. 몽타주의 Enemy Strike Window 노티파이 스테이트가 부른다. 휘두르는 중이 아니면 무시한다.
	 * 구간 동안 StrikeBone이 지나간 선분과 반경 Radius로 플레이어 몸을 판정하고, 한 구간에서 한 번만 맞힌다.
	 */
	void BeginStrikeWindow(FName StrikeBone, float Radius);

	/** 판정 구간 중 매 프레임. 지난 프레임부터 지금까지 본이 지나간 선분으로 판정한다. */
	void UpdateStrikeWindow();

	/** 판정 구간 끝. 구간 내내 맞지 않았으면 빗나감을 발행한다. */
	void EndStrikeWindow();

	/** 한 프레임짜리 타격. 몽타주의 Enemy Strike 노티파이가 부른다. 시작과 끝이 같은 판정 구간이다. */
	void Strike(FName StrikeBone, float Radius);

	/** 플레이어 머리(카메라) 위치와 플레이어 폰. VR에서 폰 위치는 트래킹 원점이라 머리 위치와 다르다. */
	static bool GetPlayerHead(const UObject* WorldContextObject, FVector& OutHead, APawn*& OutPlayerPawn);

	/** 타격 판정마다. 맞았는지와 보낸 피해를 넘긴다. */
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Attack")
	FOnEnemyStrike OnStrike;

	/** 공격이 끝나거나 멈췄을 때. */
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Attack")
	FOnEnemyAttackFinished OnAttackFinished;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 휘두르기 전에 플레이어 머리 쪽으로 도는 속도. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0", ForceUnits = "deg/s"))
	float TurnRate = 360.f;

	/** 머리 쪽과의 방향 차이가 이 각도 안이면 돌기를 멈추고 휘두른다. 휘두르는 동안에는 돌지 않아서 몸을 피하면 빗나간다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0", ForceUnits = "deg"))
	float FaceTolerance = 10.f;

	/** 판정 구간의 본 궤적과 구(빨강은 맞음, 초록은 빗나감), 플레이어 몸 캡슐(파랑)을 1초 동안 그린다. 반경과 판정 구간을 맞출 때 켠다. */
	UPROPERTY(EditAnywhere, Category = "Enemy|Debug")
	bool bDrawStrikeDebug = false;

private:
	void PlayAttackMontage();
	void FinishAttack(bool bInterrupted);
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void HandleDied();

	UPROPERTY()
	TObjectPtr<UDamageReceiver> DamageReceiver;

	/** 재생 중인 공격 몽타주. */
	UPROPERTY(VisibleInstanceOnly, Category = "Enemy|State")
	TObjectPtr<UAnimMontage> CurrentMontage;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|State", meta = (AllowPrivateAccess = "true"))
	bool bAttacking = false;

	/** 공격 중 머리 쪽으로 도는 단계인지. 몽타주를 재생하면 false. */
	bool bTurning = false;

	/** 이 시각(초) 전에는 새 공격을 시작하지 않는다. */
	double CooldownEndTime = 0.0;

	/** 타격 판정 구간 안인지. */
	bool bStrikeActive = false;

	/** 이번 판정 구간에서 이미 맞혔는지. */
	bool bStrikeHit = false;

	FName ActiveStrikeBone;
	float ActiveStrikeRadius = 0.f;

	/** 지난 판정 때 본 위치. 이 점부터 지금 위치까지의 선분으로 판정한다. */
	FVector LastStrikeLocation = FVector::ZeroVector;

	/** 이번 구간의 판정 횟수와 가장 가까웠던 거리. 빗나감 로그용. */
	int32 StrikeSamples = 0;
	float ClosestStrikeDistance = 0.f;
};
