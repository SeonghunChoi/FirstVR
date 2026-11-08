#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyHitEffects.generated.h"

class UAmmoDataAsset;
class UDamageReceiver;

/**
 * 탄에 맞은 곳의 연출(피 튀김·피격음). 같은 액터의 UDamageReceiver가 발행하는 OnDamaged를 구독하고,
 * 무엇을 재생할지는 적 데이터(HitEffect·HitSound)가 정한다. 투사체와 무기 쪽은 이 컴포넌트를 모른다.
 * 물리 반응(UHitReaction)과 나눠 둬서, 메시 물리를 만지는 컴포넌트는 여전히 하나다.
 */
UCLASS(ClassGroup = (Enemy), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UEnemyHitEffects : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyHitEffects();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleDamaged(FName BoneName, float Damage, const FHitResult& Hit, UAmmoDataAsset* Ammo);

	UPROPERTY(Transient)
	TObjectPtr<UDamageReceiver> DamageReceiver;
};
