#include "UI/EncounterResultWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Game/FirstVRGameMode.h"
#include "Player/PlayerHealth.h"

void UEncounterResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RestartButton->OnClicked.AddUniqueDynamic(this, &UEncounterResultWidget::HandleRestartClicked);

	if (NextLevelButton)
	{
		const AFirstVRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFirstVRGameMode>() : nullptr;
		const bool bHasNext = GameMode && GameMode->HasNextLevel();
		NextLevelButton->SetVisibility(bHasNext ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (bHasNext)
		{
			NextLevelButton->OnClicked.AddUniqueDynamic(this, &UEncounterResultWidget::HandleNextLevelClicked);
			if (NextLevelText)
			{
				NextLevelText->SetText(GameMode->GetNextLevelLabel());
			}
		}
	}
}

void UEncounterResultWidget::SetButtonsEnabled(bool bEnabled)
{
	RestartButton->SetIsEnabled(bEnabled);
	if (NextLevelButton)
	{
		NextLevelButton->SetIsEnabled(bEnabled);
	}
}

void UEncounterResultWidget::HandleRestartClicked()
{
	// 레벨이 다시 열릴 때까지 한 번만 누르게 한다.
	SetButtonsEnabled(false);

	AFirstVRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFirstVRGameMode>() : nullptr;
	if (!GameMode)
	{
		UE_LOG(LogPlayer, Warning, TEXT("[%s] Restart clicked, but the game mode is not AFirstVRGameMode"), *GetName());
		SetButtonsEnabled(true);
		return;
	}

	UE_LOG(LogPlayer, Log, TEXT("[%s] Restart clicked"), *GetName());
	GameMode->RestartEncounter();
}

void UEncounterResultWidget::HandleNextLevelClicked()
{
	SetButtonsEnabled(false);

	AFirstVRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFirstVRGameMode>() : nullptr;
	if (!GameMode || !GameMode->HasNextLevel())
	{
		UE_LOG(LogPlayer, Warning, TEXT("[%s] Next level clicked, but there is no next level"), *GetName());
		SetButtonsEnabled(true);
		return;
	}

	UE_LOG(LogPlayer, Log, TEXT("[%s] Next level clicked"), *GetName());
	GameMode->OpenNextLevel();
}
