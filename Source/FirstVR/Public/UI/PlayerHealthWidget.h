#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHealthWidget.generated.h"

class UPlayerHealth;
class UProgressBar;
class UTextBlock;

/**
 * 플레이어 체력 표시 위젯의 C++ 부모. 소유 플레이어 폰의 UPlayerHealth를 찾아 OnHealthChanged를 구독하고
 * 막대와 글자를 갱신한다. 배치와 모양은 자식 위젯 블루프린트(WBP)가 정한다.
 * WBP에는 이름이 HealthBar인 ProgressBar가 반드시 있어야 하고(없으면 WBP 컴파일 오류), HealthText인 TextBlock은 있으면 쓴다.
 */
UCLASS(Abstract)
class FIRSTVR_API UPlayerHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 표시할 체력 컴포넌트를 바꾼다. 보통은 생성될 때 소유 폰에서 스스로 찾는다. */
	UFUNCTION(BlueprintCallable, Category = "Player|Health Widget")
	void BindToHealth(UPlayerHealth* InHealth);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 막대와 글자를 갱신한 뒤 불린다. 맞았을 때 깜빡임 같은 연출은 WBP에서 여기에 단다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Health Widget")
	void OnHealthDisplayUpdated(float CurrentHealth, float MaxHealth, float Damage);

	/** 남은 체력 비율(0~1)을 보여 주는 막대. WBP에서 이 이름으로 놓는다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	/** "80 / 100" 글자. 없어도 된다. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

private:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Damage);

	void UnbindFromHealth();
	void UpdateDisplay(float CurrentHealth, float MaxHealth, float Damage);

	TWeakObjectPtr<UPlayerHealth> Health;
};
