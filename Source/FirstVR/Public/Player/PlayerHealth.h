#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHealth.generated.h"

class AController;
class UDamageType;

DECLARE_LOG_CATEGORY_EXTERN(LogPlayer, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerHealthChanged, float, CurrentHealth, float, MaxHealth, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

/**
 * 플레이어 체력. 폰의 OnTakeAnyDamage(언리얼 기본 데미지)를 구독해 체력을 깎고 결과를 발행만 한다.
 * 피격 피드백, 체력 표시, 실패 처리는 각자 구독한다. 죽은 뒤 피해는 무시한다.
 */
UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UPlayerHealth : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerHealth();

	/** 체력을 Damage만큼 깎는다. 0 이하이거나 이미 죽었으면 무시한다. */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void TakeHealthDamage(float Damage);

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetMaxHealth() const { return MaxHealth; }

	/** 남은 체력 비율(0~1). 체력 막대에 쓴다. */
	UFUNCTION(BlueprintPure, Category = "Player")
	float GetHealthRatio() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Player")
	bool IsDead() const { return bDead; }

	/** 체력이 바뀔 때마다. 시작할 때도 한 번(Damage 0) 발행해 표시가 처음 값을 받게 한다. */
	UPROPERTY(BlueprintAssignable, Category = "Player")
	FOnPlayerHealthChanged OnHealthChanged;

	/** 체력이 0이 됐을 때 한 번. */
	UPROPERTY(BlueprintAssignable, Category = "Player")
	FOnPlayerDied OnDied;

protected:
	virtual void BeginPlay() override;

	/** 최대 체력. 시작할 때 이 값으로 채운다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Player|State", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Player|State", meta = (AllowPrivateAccess = "true"))
	bool bDead = false;
};
