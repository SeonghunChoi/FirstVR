#include "UI/FailScreenWidget.h"

#include "Components/Button.h"
#include "Engine/World.h"
#include "Game/FirstVRGameMode.h"
#include "Player/PlayerHealth.h"

void UFailScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RestartButton->OnClicked.AddUniqueDynamic(this, &UFailScreenWidget::HandleRestartClicked);
}

void UFailScreenWidget::HandleRestartClicked()
{
	// 레벨이 다시 열릴 때까지 한 번만 누르게 한다.
	RestartButton->SetIsEnabled(false);

	AFirstVRGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFirstVRGameMode>() : nullptr;
	if (!GameMode)
	{
		UE_LOG(LogPlayer, Warning, TEXT("[%s] Restart clicked, but the game mode is not AFirstVRGameMode"), *GetName());
		RestartButton->SetIsEnabled(true);
		return;
	}

	UE_LOG(LogPlayer, Log, TEXT("[%s] Restart clicked"), *GetName());
	GameMode->RestartEncounter();
}
