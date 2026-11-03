#include "Enemy/EnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Enemy/DamageReceiver.h"
#include "Enemy/HitReaction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 메시는 바디 단위로 물리를 돌리고 총알을 받는다. Ragdoll 프로필은 Pawn 채널을 무시해서 자기 캡슐과 부딪히지 않는다.
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	DamageReceiver = CreateDefaultSubobject<UDamageReceiver>(TEXT("DamageReceiver"));
	PhysicalAnimation = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimation"));
	HitReaction = CreateDefaultSubobject<UHitReaction>(TEXT("HitReaction"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	DamageReceiver->OnDied.AddDynamic(this, &AEnemyCharacter::HandleDied);
}

void AEnemyCharacter::HandleDied()
{
	// 래그돌은 UHitReaction이 켠다. 몸은 길을 막지 않고 더 움직이지 않게만 한다.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	UE_LOG(LogEnemy, Log, TEXT("[%s] Body stopped (capsule off, movement off)"), *GetName());
}
