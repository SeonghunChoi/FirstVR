#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EncounterResultWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 구간 결과 화면(실패·승리) 위젯의 C++ 부모. '다시 시작' 버튼을 누르면 게임 모드의 RestartEncounter를 부른다.
 * '다음 레벨' 버튼(선택)은 게임 모드에 NextLevel이 있을 때만 보이고, 누르면 OpenNextLevel. 글자는 게임 모드의 NextLevelLabel.
 * 배치와 문구는 자식 WBP(WBP_FailScreen, WBP_VictoryScreen)가 정한다. 이름이 RestartButton인 Button이 반드시 있어야 한다.
 * 화면을 눈앞에 띄우고 포인터로 누르게 하는 것은 이 위젯을 담은 액터(템플릿 메뉴 복제)가 맡는다.
 * 6-5d에서 UFailScreenWidget으로 만들었고, 승리 화면과 함께 쓰려고 이름을 바꿨다(DefaultEngine.ini [CoreRedirects]).
 */
UCLASS(Abstract)
class FIRSTVR_API UEncounterResultWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** 다시 시작 버튼. WBP에서 이 이름으로 놓는다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RestartButton;

	/** 다음 레벨 버튼(선택). 게임 모드에 다음 레벨이 없으면 접는다. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> NextLevelButton;

	/** 다음 레벨 버튼 안의 글자(선택). 게임 모드의 NextLevelLabel로 채운다. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NextLevelText;

private:
	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleNextLevelClicked();

	/** 레벨이 바뀔 때까지 두 버튼을 모두 끈다(한 번만 누르게). */
	void SetButtonsEnabled(bool bEnabled);
};
