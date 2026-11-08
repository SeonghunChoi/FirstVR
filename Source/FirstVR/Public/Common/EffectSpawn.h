#pragma once

#include "CoreMinimal.h"

class UFXSystemAsset;
class USceneComponent;

/**
 * 연출 이펙트 스폰 도우미. 나이아가라(UNiagaraSystem)와 옛 캐스케이드(UParticleSystem)를 같은 칸(UFXSystemAsset)으로 받는다.
 * 유료 팩의 총구 화염처럼 캐스케이드로 된 에셋도 그대로 쓰기 위해서다. 둘 다 풀링(AutoRelease)으로 스폰한다.
 */
namespace FirstVRFX
{
	/** 월드 위치에 한 번 재생하고 끝나면 정리한다. Effect가 비어 있으면 아무것도 하지 않는다. */
	FIRSTVR_API void SpawnAtLocation(const UObject* WorldContextObject, UFXSystemAsset* Effect, const FVector& Location, const FRotator& Rotation, const FVector& Scale = FVector::OneVector);

	/** 컴포넌트에 붙여 재생한다(그 컴포넌트의 위치·방향을 따라감). */
	FIRSTVR_API void SpawnAttached(UFXSystemAsset* Effect, USceneComponent* AttachTo, const FVector& Scale = FVector::OneVector);
}
