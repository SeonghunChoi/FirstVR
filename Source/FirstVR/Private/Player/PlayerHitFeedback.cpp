#include "Player/PlayerHitFeedback.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/PlayerHealth.h"
#include "Sound/SoundBase.h"

UPlayerHitFeedback::UPlayerHitFeedback()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 번쩍임이 줄어드는 동안에만 켠다.
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPlayerHitFeedback::BeginPlay()
{
	Super::BeginPlay();

	if (UPlayerHealth* Health = GetOwner()->FindComponentByClass<UPlayerHealth>())
	{
		Health->OnHealthChanged.AddDynamic(this, &UPlayerHitFeedback::HandleHealthChanged);
	}
	else
	{
		UE_LOG(LogPlayer, Warning, TEXT("[%s] PlayerHitFeedback found no PlayerHealth"), *GetOwner()->GetName());
	}

	Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (Camera && FlashMaterial)
	{
		// 머티리얼 인스턴스를 만들어 카메라 포스트 프로세스에 넣어 둔다. 쉴 때는 가중치 0으로 꺼 둔다.
		FlashInstance = UMaterialInstanceDynamic::Create(FlashMaterial, this);
		SetFlashAmount(0.f);
	}

	UE_LOG(LogPlayer, Log, TEXT("[%s] Hit feedback ready (flash %s, haptic %s, sound %s)"),
		*GetOwner()->GetName(), *GetNameSafe(FlashInstance ? FlashMaterial.Get() : nullptr), *GetNameSafe(HitHaptic), *GetNameSafe(HitSound));
}

void UPlayerHitFeedback::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Damage)
{
	// 시작할 때 한 번 오는 알림(Damage 0)은 피격이 아니다.
	if (Damage > 0.f)
	{
		PlayHitFeedback(Damage);
	}
}

void UPlayerHitFeedback::PlayHitFeedback(float Damage)
{
	if (FlashInstance)
	{
		FlashElapsed = 0.f;
		SetFlashAmount(FlashIntensity);
		SetComponentTickEnabled(true);
	}

	PlayHaptics();

	if (HitSound)
	{
		UGameplayStatics::PlaySound2D(this, HitSound, HitSoundVolume, HitSoundPitch);
	}

	UE_LOG(LogPlayer, Log, TEXT("[%s] Hit feedback (damage %.0f)"), *GetOwner()->GetName(), Damage);
}

void UPlayerHitFeedback::PlayHaptics() const
{
	if (!HitHaptic)
	{
		return;
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PlayerController)
	{
		return;
	}

	PlayerController->PlayHapticEffect(HitHaptic, EControllerHand::Left, HapticScale);
	PlayerController->PlayHapticEffect(HitHaptic, EControllerHand::Right, HapticScale);
}

void UPlayerHitFeedback::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FlashElapsed += DeltaTime;
	const float Remaining = 1.f - FlashElapsed / FlashDuration;
	if (Remaining <= 0.f)
	{
		SetFlashAmount(0.f);
		SetComponentTickEnabled(false);
		return;
	}

	SetFlashAmount(FlashIntensity * Remaining);
}

void UPlayerHitFeedback::SetFlashAmount(float Amount)
{
	FlashAmount = Amount;
	if (!FlashInstance || !Camera)
	{
		return;
	}

	FlashInstance->SetScalarParameterValue(FlashParameterName, Amount);
	// 가중치 0이면 포스트 프로세스 패스에서 빠져 쉬는 동안 비용이 없다.
	Camera->AddOrUpdateBlendable(FlashInstance.Get(), Amount > 0.f ? 1.f : 0.f);
}
