#include "Weapon/WeaponProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Weapon/AmmoDataAsset.h"
#include "Weapon/Hittable.h"
#include "Common/EffectSpawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Weapon/WeaponTypes.h"

AWeaponProjectile::AWeaponProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 3.f;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(2.f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	// 캐릭터 캡슐(Pawn)은 지나쳐서 그 안의 메시 바디에 맞아야 본 이름이 나온다.
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SetRootComponent(Collision);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->UpdatedComponent = Collision;
	Movement->bRotationFollowsVelocity = true;
	Movement->bShouldBounce = false;
}

void AWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();

	Collision->IgnoreActorWhenMoving(GetOwner(), true);
	Movement->OnProjectileStop.AddDynamic(this, &AWeaponProjectile::HandleStop);
}

void AWeaponProjectile::Launch(UAmmoDataAsset* InAmmo, float InDamageScale)
{
	Ammo = InAmmo;
	DamageScale = InDamageScale;

	const float Speed = Ammo ? Ammo->Speed : 0.f;
	Movement->Velocity = GetActorForwardVector() * Speed;

	UE_LOG(LogWeapon, Log, TEXT("[%s] Launch (Ammo %s, Speed %.0f, DamageScale %.2f) from %s"), *GetName(), *GetNameSafe(Ammo), Speed, DamageScale, *GetActorLocation().ToCompactString());
}

void AWeaponProjectile::HandleStop(const FHitResult& Hit)
{
	UE_LOG(LogWeapon, Log, TEXT("[%s] Hit %s (%s) at %s"), *GetName(), *GetNameSafe(Hit.GetActor()), *Hit.BoneName.ToString(), *Hit.ImpactPoint.ToCompactString());

	IHittable* Hittable = Hit.GetActor() ? Hit.GetActor()->FindComponentByInterface<IHittable>() : nullptr;
	if (Hittable)
	{
		// 피해를 받는 쪽의 연출(피 튀김 등)은 그쪽이 맡는다. 투사체는 탄 데이터와 배율만 넘긴다.
		Hittable->ReceiveHit(Hit, Ammo, DamageScale);
	}
	else if (Ammo)
	{
		// 벽·바닥: 맞은 면의 법선을 이펙트의 위(+Z)로.
		const FRotator Rotation = FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator();
		FirstVRFX::SpawnAtLocation(this, Ammo->ImpactEffect, Hit.ImpactPoint, Rotation, Ammo->ImpactEffectScale);
		if (Ammo->ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Ammo->ImpactSound, Hit.ImpactPoint, Ammo->ImpactSoundVolume, FMath::FRandRange(0.9f, 1.1f));
		}
	}
	Destroy();
}
