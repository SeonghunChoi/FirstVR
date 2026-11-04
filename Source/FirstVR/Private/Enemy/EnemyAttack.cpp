#include "Enemy/EnemyAttack.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Enemy/DamageReceiver.h"
#include "Enemy/EnemyDataAsset.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerBody.h"

UEnemyAttack::UEnemyAttack()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UEnemyAttack::BeginPlay()
{
	Super::BeginPlay();

	DamageReceiver = GetOwner()->FindComponentByClass<UDamageReceiver>();
	if (DamageReceiver)
	{
		DamageReceiver->OnDied.AddDynamic(this, &UEnemyAttack::HandleDied);
	}
	else
	{
		UE_LOG(LogEnemy, Warning, TEXT("[%s] EnemyAttack found no DamageReceiver"), *GetOwner()->GetName());
	}
}

bool UEnemyAttack::CanAttack() const
{
	if (bAttacking || !DamageReceiver || DamageReceiver->IsDead())
	{
		return false;
	}
	return GetWorld()->GetTimeSeconds() >= CooldownEndTime;
}

bool UEnemyAttack::StartAttack()
{
	if (!CanAttack())
	{
		return false;
	}

	bAttacking = true;
	bTurning = true;
	SetComponentTickEnabled(true);

	UE_LOG(LogEnemy, Log, TEXT("[%s] Attack started (turning to target)"), *GetOwner()->GetName());
	return true;
}

void UEnemyAttack::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bTurning)
	{
		SetComponentTickEnabled(false);
		return;
	}

	FVector Head;
	APawn* PlayerPawn = nullptr;
	if (!GetPlayerHead(this, Head, PlayerPawn))
	{
		PlayAttackMontage();
		return;
	}

	AActor* Owner = GetOwner();
	const float TargetYaw = (Head - Owner->GetActorLocation()).GetSafeNormal2D().Rotation().Yaw;
	FRotator Rotation = Owner->GetActorRotation();
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(Rotation.Yaw, TargetYaw);

	if (FMath::Abs(DeltaYaw) <= FaceTolerance)
	{
		PlayAttackMontage();
		return;
	}

	const float MaxStep = TurnRate * DeltaTime;
	Rotation.Yaw += FMath::Clamp(DeltaYaw, -MaxStep, MaxStep);
	Owner->SetActorRotation(Rotation);
}

void UEnemyAttack::PlayAttackMontage()
{
	bTurning = false;
	SetComponentTickEnabled(false);

	const UEnemyDataAsset* Data = DamageReceiver ? DamageReceiver->GetEnemyData() : nullptr;
	if (!Data || Data->AttackMontages.IsEmpty())
	{
		UE_LOG(LogEnemy, Warning, TEXT("[%s] No attack montage in enemy data"), *GetOwner()->GetName());
		FinishAttack(true);
		return;
	}

	UAnimMontage* Montage = Data->AttackMontages[FMath::RandHelper(Data->AttackMontages.Num())];
	const USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
	if (!Montage || !AnimInstance || AnimInstance->Montage_Play(Montage) <= 0.f)
	{
		UE_LOG(LogEnemy, Warning, TEXT("[%s] Could not play attack montage %s"), *GetOwner()->GetName(), *GetNameSafe(Montage));
		FinishAttack(true);
		return;
	}

	CurrentMontage = Montage;
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UEnemyAttack::HandleMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);

	UE_LOG(LogEnemy, Log, TEXT("[%s] Attack swing %s"), *GetOwner()->GetName(), *Montage->GetName());
}

void UEnemyAttack::BeginStrikeWindow(FName StrikeBone, float Radius)
{
	if (!bAttacking || bTurning)
	{
		return;
	}

	const USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh)
	{
		return;
	}

	bStrikeActive = true;
	bStrikeHit = false;
	ActiveStrikeBone = StrikeBone;
	ActiveStrikeRadius = Radius;
	StrikeSamples = 0;
	ClosestStrikeDistance = TNumericLimits<float>::Max();
	LastStrikeLocation = Mesh->GetSocketLocation(StrikeBone);
	UpdateStrikeWindow();
}

void UEnemyAttack::UpdateStrikeWindow()
{
	if (!bStrikeActive || bStrikeHit)
	{
		return;
	}

	const USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	FVector Head;
	APawn* PlayerPawn = nullptr;
	if (!Mesh || !GetPlayerHead(this, Head, PlayerPawn) || !PlayerPawn)
	{
		return;
	}

	// 지난 샘플부터 지금까지 본이 지나간 선분으로 판정한다. 빠른 동작이 프레임 사이에 몸을 건너뛰어도 맞는다.
	const FVector StrikeLocation = Mesh->GetSocketLocation(ActiveStrikeBone);
	const UPlayerBody* Body = PlayerPawn->FindComponentByClass<UPlayerBody>();
	const float Distance = Body ? Body->GetDistanceToSurface(LastStrikeLocation, StrikeLocation)
		: FMath::PointDistToSegment(Head, LastStrikeLocation, StrikeLocation);
	const bool bHit = Distance <= ActiveStrikeRadius;
	++StrikeSamples;
	ClosestStrikeDistance = FMath::Min(ClosestStrikeDistance, Distance);

	if (bDrawStrikeDebug)
	{
		const FColor Color = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(GetWorld(), LastStrikeLocation, StrikeLocation, Color, false, 1.f, 0, 2.f);
		DrawDebugSphere(GetWorld(), StrikeLocation, ActiveStrikeRadius, 12, Color, false, 1.f);
		if (Body)
		{
			DrawDebugCapsule(GetWorld(), Body->GetComponentLocation(), Body->GetScaledCapsuleHalfHeight(), Body->GetScaledCapsuleRadius(), Body->GetComponentQuat(), FColor::Blue, false, 1.f);
		}
	}
	LastStrikeLocation = StrikeLocation;

	if (!bHit)
	{
		return;
	}

	// 한 번의 판정 구간에서는 한 번만 맞힌다.
	bStrikeHit = true;
	const UEnemyDataAsset* Data = DamageReceiver ? DamageReceiver->GetEnemyData() : nullptr;
	const float Damage = Data ? Data->AttackDamage : 0.f;
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	UGameplayStatics::ApplyDamage(PlayerPawn, Damage, OwnerPawn ? OwnerPawn->GetController() : nullptr, GetOwner(), UDamageType::StaticClass());
	UE_LOG(LogEnemy, Log, TEXT("[%s] Strike %s hit %s (%s distance %.0f / %.0f, sample %d), damage %.0f"), *GetOwner()->GetName(), *ActiveStrikeBone.ToString(),
		*PlayerPawn->GetName(), Body ? TEXT("body") : TEXT("head"), Distance, ActiveStrikeRadius, StrikeSamples, Damage);
	OnStrike.Broadcast(true, Damage);
}

void UEnemyAttack::EndStrikeWindow()
{
	if (!bStrikeActive)
	{
		return;
	}

	bStrikeActive = false;
	if (!bStrikeHit)
	{
		UE_LOG(LogEnemy, Log, TEXT("[%s] Strike %s missed (closest distance %.0f / %.0f, %d samples)"), *GetOwner()->GetName(), *ActiveStrikeBone.ToString(),
			ClosestStrikeDistance, ActiveStrikeRadius, StrikeSamples);
		OnStrike.Broadcast(false, 0.f);
	}
}

void UEnemyAttack::Strike(FName StrikeBone, float Radius)
{
	BeginStrikeWindow(StrikeBone, Radius);
	EndStrikeWindow();
}

void UEnemyAttack::CancelAttack()
{
	if (!bAttacking)
	{
		return;
	}

	const USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr; AnimInstance && CurrentMontage)
	{
		AnimInstance->Montage_Stop(0.2f, CurrentMontage);
	}
	FinishAttack(true);
}

void UEnemyAttack::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == CurrentMontage)
	{
		FinishAttack(bInterrupted);
	}
}

void UEnemyAttack::FinishAttack(bool bInterrupted)
{
	if (!bAttacking)
	{
		return;
	}

	bAttacking = false;
	bTurning = false;
	bStrikeActive = false;
	CurrentMontage = nullptr;
	SetComponentTickEnabled(false);

	const UEnemyDataAsset* Data = DamageReceiver ? DamageReceiver->GetEnemyData() : nullptr;
	const float Cooldown = Data ? Data->AttackCooldown : 0.f;
	CooldownEndTime = GetWorld()->GetTimeSeconds() + Cooldown;

	UE_LOG(LogEnemy, Log, TEXT("[%s] Attack %s, cooldown %.1fs"), *GetOwner()->GetName(), bInterrupted ? TEXT("interrupted") : TEXT("finished"), Cooldown);
	OnAttackFinished.Broadcast();
}

void UEnemyAttack::HandleDied()
{
	CancelAttack();
}

bool UEnemyAttack::GetPlayerHead(const UObject* WorldContextObject, FVector& OutHead, APawn*& OutPlayerPawn)
{
	APlayerController* Player = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	const APlayerCameraManager* Camera = Player ? Player->PlayerCameraManager.Get() : nullptr;
	if (!Camera)
	{
		return false;
	}

	OutHead = Camera->GetCameraLocation();
	OutPlayerPawn = Player->GetPawn();
	return true;
}
