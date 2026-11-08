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
	LegIKAlpha = LimpAlpha * (1.f - GetSlotMontageGlobalWeight(MontageSlotName));

	// 다친 다리가 앞으로 나가는 동안 더 느리게: 걷기 애니메이션의 스윙 커브(직전 포즈 값)로 두 재생 속도를 섞는다
	InjuredFootSwing = 0.f;
	if (InjuredLeg != ELegSide::None)
	{
		InjuredFootSwing = FMath::Clamp(GetCurveValue(InjuredLeg == ELegSide::Left ? LeftFootSwingCurve : RightFootSwingCurve), 0.f, 1.f);
	}
	const float LimpRate = FMath::Lerp(LimpPlayRate, LimpSwingPlayRate, InjuredFootSwing);
	LocomotionPlayRate = FMath::Lerp(1.f, LimpRate, LimpAlpha);
}

void UEnemyAnimInstance::HandleLimpStarted(ELegSide InInjuredLeg)
{
	bLimping = true;
	InjuredLeg = InInjuredLeg;

	UE_LOG(LogEnemy, Log, TEXT("[%s] Limp pose (%s leg, play rate %.2f / swing %.2f)"), *GetNameSafe(GetOwningActor()),
		*UEnum::GetDisplayValueAsText(InjuredLeg).ToString(), LimpPlayRate, LimpSwingPlayRate);
}
