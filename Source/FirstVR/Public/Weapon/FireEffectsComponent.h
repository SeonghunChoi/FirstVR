#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FireEffectsComponent.generated.h"

class UFXSystemAsset;
class USceneComponent;
class USoundBase;

/**
 * 발사 연출 부품. 코어의 OnFired(실제로 탄이 나갔을 때)를 구독해 총구 화염·탄피·총소리를 낸다.
 * 발사 흐름의 단계가 아니다. 코어와 단계는 이 부품을 모른다(레이저 사이트와 같은 부품).
 * 에셋은 무기 블루프린트에서 정하고, 유료 에셋으로 꾸민 자식 BP가 덮어쓴다. 비워 둔 칸은 재생하지 않는다.
 */
UCLASS(ClassGroup = (Weapon), Blueprintable, meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UFireEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFireEffectsComponent();

	/** 연출을 한 번 재생한다. 평소에는 OnFired가 부르고, 시험할 때 블루프린트에서 직접 불러도 된다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Effects")
	void PlayFireEffects();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 총구 화염. 나이아가라 또는 캐스케이드. Muzzle에 붙여 재생해 총을 따라 움직인다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	TObjectPtr<UFXSystemAsset> MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	FVector MuzzleEffectScale = FVector::OneVector;

	/** 총구 화염과 총소리 위치가 되는 Scene Component의 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	FName MuzzleComponentName = TEXT("Muzzle");

	/** 탄피 배출. 이름이 ShellEjectComponentName인 기준점이 없으면 재생하지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	TObjectPtr<UFXSystemAsset> ShellEjectEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	FName ShellEjectComponentName = TEXT("ShellEject");

	/** 총소리. 총구 위치에서 3D로 재생한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects", meta = (ClampMin = "0.0"))
	float FireSoundVolume = 1.f;

	/** 연발에서 같은 소리가 기계적으로 반복되지 않게 발마다 피치를 이 범위에서 고른다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects", meta = (ClampMin = "0.1"))
	FVector2D FireSoundPitchRange = FVector2D(0.95, 1.05);

private:
	UFUNCTION()
	void HandleFired(float Recoil);

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> ShellEject;
};
