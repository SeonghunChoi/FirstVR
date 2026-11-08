#include "Common/EffectSpawn.h"

#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"

void FirstVRFX::SpawnAtLocation(const UObject* WorldContextObject, UFXSystemAsset* Effect, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
	if (!Effect || !WorldContextObject)
	{
		return;
	}

	if (UNiagaraSystem* Niagara = Cast<UNiagaraSystem>(Effect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContextObject, Niagara, Location, Rotation, Scale, true, true, ENCPoolMethod::AutoRelease);
	}
	else if (UParticleSystem* Cascade = Cast<UParticleSystem>(Effect))
	{
		UGameplayStatics::SpawnEmitterAtLocation(WorldContextObject, Cascade, Location, Rotation, Scale, true, EPSCPoolMethod::AutoRelease);
	}
}

void FirstVRFX::SpawnAttached(UFXSystemAsset* Effect, USceneComponent* AttachTo, const FVector& Scale)
{
	if (!Effect || !AttachTo)
	{
		return;
	}

	if (UNiagaraSystem* Niagara = Cast<UNiagaraSystem>(Effect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(Niagara, AttachTo, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, Scale,
			EAttachLocation::SnapToTarget, true, ENCPoolMethod::AutoRelease);
	}
	else if (UParticleSystem* Cascade = Cast<UParticleSystem>(Effect))
	{
		UGameplayStatics::SpawnEmitterAttached(Cascade, AttachTo, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, Scale,
			EAttachLocation::SnapToTarget, true, EPSCPoolMethod::AutoRelease);
	}
}
