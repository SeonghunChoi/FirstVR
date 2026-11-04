#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHitFeedback.generated.h"

class UCameraComponent;
class UHapticFeedbackEffect_Base;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPlayerHealth;
class USoundBase;

/**
 * 플레이어 피격 피드백. 같은 폰의 UPlayerHealth::OnHealthChanged를 구독해 피해가 있을 때
 * 화면 가장자리를 붉게 물들이고(카메라 포스트 프로세스 머티리얼), 양손을 진동시키고, 피격음을 낸다.
 * 체력은 이 컴포넌트를 모른다. 빼도 체력과 실패 처리는 그대로 동작한다.
 */
UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UPlayerHitFeedback : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerHitFeedback();

	/** 피드백을 한 번 재생한다. 체력 구독 없이 시험할 때도 쓴다. */
	UFUNCTION(BlueprintCallable, Category = "Player|Hit Feedback")
	void PlayHitFeedback(float Damage);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	/** 화면 가장자리를 물들이는 포스트 프로세스 머티리얼. 세기 스칼라 파라미터를 가져야 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Flash")
	TObjectPtr<UMaterialInterface> FlashMaterial;

	/** 머티리얼에서 세기(0~1)를 받는 스칼라 파라미터 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Flash")
	FName FlashParameterName = TEXT("Intensity");

	/** 맞은 순간의 세기. 이 값에서 0까지 줄어든다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Flash", meta = (ClampMin = "0.0"))
	float FlashIntensity = 1.f;

	/** 세기가 0이 될 때까지 걸리는 시간(초). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Flash", meta = (ClampMin = "0.05"))
	float FlashDuration = 0.4f;

	/** 양손에 함께 재생할 진동. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Haptic")
	TObjectPtr<UHapticFeedbackEffect_Base> HitHaptic;

	/** 진동 세기(0~1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Haptic", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HapticScale = 1.f;

	/** 피격음. 플레이어 자신의 소리라 위치 없이(2D) 재생한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Sound")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Sound", meta = (ClampMin = "0.0"))
	float HitSoundVolume = 1.f;

	/** 재생 피치 배율. 임시로 다른 소리를 쓸 때 낮춰서 구분한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Hit Feedback|Sound", meta = (ClampMin = "0.1"))
	float HitSoundPitch = 1.f;

private:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Damage);

	void PlayHaptics() const;
	void SetFlashAmount(float Amount);

	/** 지금 화면에 적용된 세기. */
	UPROPERTY(VisibleInstanceOnly, Category = "Player|Hit Feedback|State")
	float FlashAmount = 0.f;

	float FlashElapsed = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FlashInstance;
};
