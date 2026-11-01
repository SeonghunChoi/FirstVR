#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponProjectile.generated.h"

class UAmmoDataAsset;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * 발사된 탄. 탄 데이터와 DamageScale을 들고 날아가 맞은 액터의 IHittable 컴포넌트에 전달한다.
 * 무기 시스템과 적 시스템이 만나는 유일한 지점이다.
 */
UCLASS(Abstract)
class FIRSTVR_API AWeaponProjectile : public AActor
{
	GENERATED_BODY()

public:
	AWeaponProjectile();

	/** 스폰 직후 코어가 부른다. 탄 데이터의 Speed로 앞 방향 속도를 정한다. */
	void Launch(UAmmoDataAsset* InAmmo, float InDamageScale);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleStop(const FHitResult& Hit);

	/** 충돌 판정. 루트 컴포넌트다. */
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> Collision;

	/** 보이는 모양. 충돌은 Collision이 맡는다. 메시는 블루프린트에서 지정한다. */
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> Movement;

	/** 이 탄의 데이터. 피격 쪽이 피해 계산에 쓴다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile|State", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAmmoDataAsset> Ammo;

	/** 발사 흐름에서 정해진 피해 배율. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile|State", meta = (AllowPrivateAccess = "true"))
	float DamageScale = 1.f;
};
