#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FailScreenWidget.generated.h"

class UButton;

/**
 * 실패 화면 위젯의 C++ 부모. '다시 시작' 버튼을 누르면 게임 모드의 RestartEncounter를 부른다.
 * 배치와 문구는 자식 WBP가 정한다. 이름이 RestartButton인 Button이 반드시 있어야 한다.
 * 화면을 눈앞에 띄우고 포인터로 누르게 하는 것은 이 위젯을 담은 액터(템플릿 메뉴 복제)가 맡는다.
 */
UCLASS(Abstract)
class FIRSTVR_API UFailScreenWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** 다시 시작 버튼. WBP에서 이 이름으로 놓는다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RestartButton;

private:
	UFUNCTION()
	void HandleRestartClicked();
};
