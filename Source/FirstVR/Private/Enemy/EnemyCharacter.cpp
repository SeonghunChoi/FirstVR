#include "Enemy/EnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Enemy/AI/EnemyAIController.h"
#include "Enemy/DamageReceiver.h"
#include "Enemy/HitReaction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 컨트롤러가 바라보는 방향으로 순간 회전하지 않고, 움직이는 방향으로 RotationRate만큼 돌며 걷는다.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 270.f, 0.f);

	// 레벨에 배치하든 웨이브가 스폰하든 적의 판단(State Tree)을 돌리는 컨트롤러가 붙는다. 블루프린트 자식이 컨트롤러 BP로 바꾼다.
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 메시는 바디 단위로 물리를 돌리고 총알을 받는다. Ragdoll 프로필은 Pawn 채널을 무시해서 자기 캡슐과 부딪히지 않는다.
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	DamageReceiver = CreateDefaultSubobject<UDamageReceiver>(TEXT("DamageReceiver"));
	PhysicalAnimation = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimation"));
	HitReaction = CreateDefaultSubobject<UHitReaction>(TEXT("HitReaction"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	DamageReceiver->OnLimpStarted.AddDynamic(this, &AEnemyCharacter::HandleLimpStarted);
	DamageReceiver->OnDied.AddDynamic(this, &AEnemyCharacter::HandleDied);
	ApplyWalkSpeed();
}

void AEnemyCharacter::ApplyWalkSpeed()
{
	const UEnemyDataAsset* Data = DamageReceiver->GetEnemyData();
	if (!Data)
	{
		return;
	}

	const float Scale = DamageReceiver->IsLimping() ? Data->LimpSpeedScale : 1.f;
	GetCharacterMovement()->MaxWalkSpeed = Data->WalkSpeed * Scale;

	UE_LOG(LogEnemy, Log, TEXT("[%s] Walk speed %.0f (x%.2f)"), *GetName(), GetCharacterMovement()->MaxWalkSpeed, Scale);
}

void AEnemyCharacter::HandleLimpStarted(ELegSide InjuredLeg)
{
	ApplyWalkSpeed();
}

void AEnemyCharacter::HandleDied()
{
	// 래그돌은 UHitReaction이 켠다. 몸은 길을 막지 않고 더 움직이지 않게 하고, 판단을 멈춘다.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	UE_LOG(LogEnemy, Log, TEXT("[%s] Body stopped (capsule off, movement off)"), *GetName());

	// 컨트롤러를 떼고 없앤다. 컨트롤러는 폰을 놓을 때 State Tree를 멈춘다.
	DetachFromControllerPendingDestroy();
}
