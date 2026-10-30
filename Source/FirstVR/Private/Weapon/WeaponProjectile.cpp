#include "Weapon/WeaponProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Weapon/AmmoDataAsset.h"
#include "Weapon/WeaponTypes.h"

AWeaponProjectile::AWeaponProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 3.f;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(2.f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
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

	UE_LOG(LogWeapon, Log, TEXT("[%s] Launch (Ammo %s, Speed %.0f, DamageScale %.2f)"), *GetName(), *GetNameSafe(Ammo), Speed, DamageScale);
}

void AWeaponProjectile::HandleStop(const FHitResult& Hit)
{
	UE_LOG(LogWeapon, Log, TEXT("[%s] Hit %s"), *GetName(), *GetNameSafe(Hit.GetActor()));
	Destroy();
}
