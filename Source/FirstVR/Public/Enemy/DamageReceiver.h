#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/EnemyDataAsset.h"
#include "Weapon/Hittable.h"
#include "DamageReceiver.generated.h"

class UAmmoDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(LogEnemy, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnEnemyDamaged, FName, BoneName, float, Damage, const FHitResult&, Hit, UAmmoDataAsset*, Ammo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyLimpStarted, ELegSide, InjuredLeg);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);

/**
 * 피격을 받아 피해를 계산하고 체력, 다리 누적 피해, 절뚝임, 사망 상태를 관리한다.
 * 결과는 델리게이트로 발행만 한다. 피격 반응, 애니메이션 상태, 사망 연출은 각자 구독한다.
 */
UCLASS(ClassGroup = (Enemy), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UDamageReceiver : public UActorComponent, public IHittable
{
	GENERATED_BODY()

public:
	UDamageReceiver();

	virtual void ReceiveHit(const FHitResult& Hit, UAmmoDataAsset* Ammo, float DamageScale) override;

	/** 이 본에 이 탄이 맞았을 때의 최종 피해. BaseDamage × 본 배율 × DamageScale × min(1, Penetration / Armor). */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	float CalculateDamage(FName BoneName, const UAmmoDataAsset* Ammo, float DamageScale) const;

	/** 피해를 받을 때마다. 사망한 뒤에는 발행하지 않는다. */
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyDamaged OnDamaged;

	/** 다리 누적 피해가 처음 LimpThreshold 이상이 됐을 때 한 번. 그 탄이 맞은 쪽 다리를 넘긴다. */
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyLimpStarted OnLimpStarted;

	bool IsLimping() const { return bLimping; }
	const UEnemyDataAsset* GetEnemyData() const { return EnemyData; }
	ELegSide GetInjuredLeg() const { return InjuredLeg; }

	/** 체력이 0이 됐을 때 한 번. */
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyDied OnDied;

protected:
	virtual void BeginPlay() override;

	/** 이 적 종류의 데이터. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UEnemyDataAsset> EnemyData;

private:
	/** 현재 체력. BeginPlay에서 MaxHealth로 채운다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|State", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 0.f;

	/** 좌우 다리 본에 들어간 최종 피해의 합. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|State", meta = (AllowPrivateAccess = "true"))
	float LegDamage = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|State", meta = (AllowPrivateAccess = "true"))
	bool bLimping = false;

	/** 절뚝임을 시작시킨 탄이 맞은 쪽 다리. 절뚝이기 전에는 None. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|State", meta = (AllowPrivateAccess = "true"))
	ELegSide InjuredLeg = ELegSide::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|State", meta = (AllowPrivateAccess = "true"))
	bool bDead = false;
};
