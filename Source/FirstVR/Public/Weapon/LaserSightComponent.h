#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LaserSightComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;

/**
 * 조준 보조 부품. 켜져 있는 동안 발사 원점(Muzzle) 앞으로 트레이스해서 맞은 곳까지 나이아가라 빔을 그린다.
 * 발사 흐름의 단계가 아니다. 코어와 단계는 이 부품을 모른다.
 * 언제 켤지는 블루프린트 자식이 템플릿 잡기 컴포넌트를 구독해 SetLaserEnabled로 알려 준다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API ULaserSightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULaserSightComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 레이저를 켜거나 끈다. 켜져 있는 동안만 틱을 돈다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Laser")
	void SetLaserEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Weapon|Laser")
	bool IsLaserEnabled() const { return bLaserEnabled; }

	/** 같은 액터에서 이름이 GripComponentName인 Scene Component를 찾는다. 없으면 None. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Laser")
	USceneComponent* FindGripComponent() const;

protected:
	virtual void BeginPlay() override;

	/** 빔 나이아가라 시스템. 시작·끝 위치를 받는 User Parameter(Position 타입)가 있어야 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Laser")
	TObjectPtr<UNiagaraSystem> BeamSystem;

	/** 맞은 곳이 없을 때 빔 길이. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Laser", meta = (ClampMin = "1.0", ForceUnits = "cm"))
	float MaxDistance = 3000.f;

	/** 빔을 시작할 Scene Component(발사 원점)의 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Laser")
	FName MuzzleComponentName = TEXT("Muzzle");

	/** 레이저를 켜고 끌 기준이 되는 잡기 컴포넌트의 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Laser")
	FName GripComponentName = TEXT("Grip");

	/** 빔 시작 위치(월드)를 받는 User Parameter 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Laser")
	FName BeamStartParameter = TEXT("BeamStart");

	/** 빔 끝 위치(월드)를 받는 User Parameter 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Laser")
	FName BeamEndParameter = TEXT("BeamEnd");

private:
	/** 트레이스해서 빔의 시작·끝 위치를 갱신한다. */
	void UpdateBeam();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Laser|State", meta = (AllowPrivateAccess = "true"))
	bool bLaserEnabled = false;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> Beam;
};
