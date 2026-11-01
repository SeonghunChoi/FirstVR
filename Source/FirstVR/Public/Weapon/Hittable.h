#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Hittable.generated.h"

class UAmmoDataAsset;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UHittable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 투사체에 맞을 수 있는 대상. 투사체는 맞은 액터에서 이 인터페이스를 구현한 컴포넌트를 찾아 ReceiveHit을 부른다.
 * 무기 쪽은 이 인터페이스만 알고, 받는 쪽(적 등)의 클래스는 모른다. 피해 계산은 받는 쪽이 한다.
 */
class FIRSTVR_API IHittable
{
	GENERATED_BODY()

public:
	/** 투사체가 맞았을 때 부른다. Hit.BoneName에 맞은 본이 들어 있다(피직스 바디가 있는 본만). */
	virtual void ReceiveHit(const FHitResult& Hit, UAmmoDataAsset* Ammo, float DamageScale) = 0;
};
