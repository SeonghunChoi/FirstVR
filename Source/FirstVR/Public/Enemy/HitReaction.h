#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "HitReaction.generated.h"

class UAmmoDataAsset;
class USkeletalMeshComponent;

/**
 * 물리 피격 반응. UDamageReceiver의 OnDamaged를 구독해 맞은 본 아래만 잠깐 물리로 돌리고, RecoverTime 동안 애니메이션으로 되돌린다.
 * 물리로 도는 동안 같은 액터의 UPhysicalAnimationComponent가 바디를 애니메이션 자세 쪽으로 당긴다.
 * OnDied를 받으면 진행 중인 반응을 멈추고 모터를 끈 전신 래그돌로 넘어간다. 메시의 물리는 이 컴포넌트만 만진다.
 */
UCLASS(ClassGroup = (Enemy), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UHitReaction : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitReaction();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** BoneName 아래 바디를 물리로 돌리고 맞은 바디에 Impulse(속도 변화, cm/s)를 준다. 이미 도는 반응에 포함되면 그 반응을 다시 시작한다. 래그돌 뒤에는 무시한다. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Reaction")
	void ApplyPhysicalHit(FName BoneName, FVector Impulse);

	/** 진행 중인 반응을 멈추고 모터를 끈 채 모든 바디를 물리로 돌린다. 한 번만 동작한다. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Reaction")
	void StartRagdoll();

protected:
	virtual void BeginPlay() override;

	/** 물리 바디를 애니메이션 자세로 당기는 세기. 월드 기준(bIsLocalSimulation 끔)이어야 골반을 맞아도 쓰러지지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Reaction")
	FPhysicalAnimationData PhysicalAnimationData;

	/** 물리에서 애니메이션으로 완전히 돌아오는 시간. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Reaction", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float RecoverTime = 0.4f;

private:
	UFUNCTION()
	void HandleDamaged(FName BoneName, float Damage, const FHitResult& Hit, UAmmoDataAsset* Ammo);

	UFUNCTION()
	void HandleDied();

	void StopReaction(FName RootBone);

	/** 전신 래그돌로 넘어갔는지. */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Enemy|Reaction")
	bool bRagdoll = false;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimation;

	/** 물리로 도는 중인 반응. 키는 반응의 시작 본, 값은 지난 시간. */
	TMap<FName, float> ActiveReactions;
};
