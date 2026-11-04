#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Enemy/EnemyDataAsset.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 적 애니메이션 블루프린트의 부모. UDamageReceiver의 OnLimpStarted를 구독해 절뚝임 상태를 들고,
 * 애님 그래프가 그대로 꽂아 쓸 수 있게 이동 속도와 골반·발 IK 목표의 오프셋(컴포넌트 공간)을 매 프레임 계산한다.
 */
UCLASS()
class FIRSTVR_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** 땅 위 이동 속도(cm/s). 블렌드 스페이스(Idle·걷기·달리기)의 가로축에 꽂는다. */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Enemy|Movement")
	float GroundSpeed = 0.f;

	/** 절뚝임 자세에서 골반을 내리는 거리. 성한 다리는 Leg IK가 발을 붙잡아 무릎이 굽는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Limp", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float PelvisDrop = 6.f;

	/** 다친 발의 IK 목표를 옮기는 양(컴포넌트 공간, cm). 들어 올리면 다친 무릎이 굽는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Limp")
	FVector InjuredFootOffset = FVector(0.f, 0.f, 12.f);

	/** 절뚝임 자세로 넘어가는 시간. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Limp", meta = (ClampMin = "0.01", ForceUnits = "s"))
	float LimpBlendTime = 0.3f;

	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Enemy|Limp")
	bool bLimping = false;

	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Enemy|Limp")
	ELegSide InjuredLeg = ELegSide::None;

	/** 절뚝임 자세 비율. LimpBlendTime 동안 0 → 1. */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Enemy|Limp")
	float LimpAlpha = 0.f;

	/** pelvis에 더할 이동량. */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Enemy|Limp")
	FVector PelvisOffset = FVector::ZeroVector;

	/** ik_foot_l에 더할 이동량. 왼쪽이 다친 다리일 때만 0이 아니다. */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Enemy|Limp")
	FVector LeftFootOffset = FVector::ZeroVector;

	/** ik_foot_r에 더할 이동량. 오른쪽이 다친 다리일 때만 0이 아니다. */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Enemy|Limp")
	FVector RightFootOffset = FVector::ZeroVector;

	/** 공격 몽타주가 재생되는 슬롯. 이 슬롯의 몽타주가 섞이는 만큼 Leg IK를 끈다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Limp")
	FName MontageSlotName = TEXT("DefaultSlot");

	/**
	 * Leg IK 노드의 Alpha. 절뚝일 때만 켜고(LimpAlpha), 몽타주가 재생되는 동안은 그 가중치만큼 끈다.
	 * 공격 애니메이션은 ik_foot 본이 발을 따라가지 않아서, 켜 두면 차올린 발이 바닥으로 끌려 내려간다.
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Enemy|Limp")
	float LegIKAlpha = 0.f;

private:
	UFUNCTION()
	void HandleLimpStarted(ELegSide InInjuredLeg);
};
