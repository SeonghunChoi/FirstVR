#include "Enemy/EnemyAnimInstance.h"

#include "Enemy/DamageReceiver.h"
#include "GameFramework/Pawn.h"

void UEnemyAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	AActor* Owner = GetOwningActor();
	UDamageReceiver* Receiver = Owner ? Owner->FindComponentByClass<UDamageReceiver>() : nullptr;
	if (!Receiver)
	{
		UE_LOG(LogEnemy, Warning, TEXT("[%s] EnemyAnimInstance found no DamageReceiver"), *GetNameSafe(Owner));
		return;
	}

	Receiver->OnLimpStarted.AddDynamic(this, &UEnemyAnimInstance::HandleLimpStarted);
	if (Receiver->IsLimping())
	{
		HandleLimpStarted(Receiver->GetInjuredLeg());
	}
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const APawn* Pawn = TryGetPawnOwner();
	GroundSpeed = Pawn ? Pawn->GetVelocity().Size2D() : 0.f;

	LimpAlpha = FMath::FInterpConstantTo(LimpAlpha, bLimping ? 1.f : 0.f, DeltaSeconds, 1.f / LimpBlendTime);
	PelvisOffset = FVector(0.f, 0.f, -PelvisDrop * LimpAlpha);
	LeftFootOffset = InjuredLeg == ELegSide::Left ? InjuredFootOffset * LimpAlpha : FVector::ZeroVector;
	RightFootOffset = InjuredLeg == ELegSide::Right ? InjuredFootOffset * LimpAlpha : FVector::ZeroVector;
}

void UEnemyAnimInstance::HandleLimpStarted(ELegSide InInjuredLeg)
{
	bLimping = true;
	InjuredLeg = InInjuredLeg;

	UE_LOG(LogEnemy, Log, TEXT("[%s] Limp pose (%s leg)"), *GetNameSafe(GetOwningActor()), *UEnum::GetDisplayValueAsText(InjuredLeg).ToString());
}
