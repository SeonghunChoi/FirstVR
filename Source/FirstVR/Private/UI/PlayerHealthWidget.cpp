#include "UI/PlayerHealthWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Player/PlayerHealth.h"

void UPlayerHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 위젯 디자이너 미리보기에서는 폰이 없다.
	if (IsDesignTime())
	{
		return;
	}

	// Widget Component가 만든 위젯의 소유 플레이어는 로컬 플레이어다. 그 폰의 체력을 표시한다.
	const APawn* Pawn = GetOwningPlayerPawn();
	UPlayerHealth* PawnHealth = Pawn ? Pawn->FindComponentByClass<UPlayerHealth>() : nullptr;
	if (!PawnHealth)
	{
		UE_LOG(LogPlayer, Warning, TEXT("[%s] Health widget found no PlayerHealth on %s"), *GetName(), *GetNameSafe(Pawn));
		return;
	}

	BindToHealth(PawnHealth);
}

void UPlayerHealthWidget::NativeDestruct()
{
	UnbindFromHealth();
	Super::NativeDestruct();
}

void UPlayerHealthWidget::BindToHealth(UPlayerHealth* InHealth)
{
	UnbindFromHealth();

	Health = InHealth;
	if (!InHealth)
	{
		return;
	}

	InHealth->OnHealthChanged.AddUniqueDynamic(this, &UPlayerHealthWidget::HandleHealthChanged);
	// 체력 컴포넌트의 BeginPlay가 먼저 지나갔으면 시작 알림을 놓쳤으니 지금 값을 바로 그린다.
	// 아직이면 0으로 그렸다가 시작 알림으로 다시 그린다.
	UpdateDisplay(InHealth->GetCurrentHealth(), InHealth->GetMaxHealth(), 0.f);

	UE_LOG(LogPlayer, Log, TEXT("[%s] Health widget bound to %s"), *GetName(), *GetNameSafe(InHealth->GetOwner()));
}

void UPlayerHealthWidget::UnbindFromHealth()
{
	if (UPlayerHealth* OldHealth = Health.Get())
	{
		OldHealth->OnHealthChanged.RemoveDynamic(this, &UPlayerHealthWidget::HandleHealthChanged);
	}
	Health.Reset();
}

void UPlayerHealthWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Damage)
{
	UpdateDisplay(CurrentHealth, MaxHealth, Damage);
}

void UPlayerHealthWidget::UpdateDisplay(float CurrentHealth, float MaxHealth, float Damage)
{
	const float Ratio = MaxHealth > 0.f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f) : 0.f;
	HealthBar->SetPercent(Ratio);

	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth)));
	}

	OnHealthDisplayUpdated(CurrentHealth, MaxHealth, Damage);
}
