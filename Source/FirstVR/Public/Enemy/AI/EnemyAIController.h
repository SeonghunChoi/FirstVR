#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UStateTreeAIComponent;

/**
 * 적의 판단. State Tree를 돌리는 AI 컨트롤러다.
 * 어떤 State Tree 에셋을 쓸지는 블루프린트 자식의 StateTreeAI 컴포넌트에서 정한다.
 * State Tree는 폰을 잡고 게임이 시작된 뒤에 시작하고, 폰을 놓으면(사망 포함) 멈춘다.
 */
UCLASS()
class FIRSTVR_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	/**
	 * 구간 규칙으로 판단을 멈춘다(예: 플레이어 실패). 진행 중인 이동과 공격도 끝나고, 다시 시작하지 않는다.
	 * 레벨을 다시 열면 새 컨트롤러가 생기므로 되돌리는 함수는 두지 않는다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void HaltLogic(const FString& Reason);

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;

private:
	/** 폰을 잡았고 게임이 시작됐으면 State Tree를 시작한다. 둘 중 늦게 오는 쪽에서 시작된다. */
	void TryStartLogic();

	/** HaltLogic으로 멈췄는지. 멈춘 뒤에는 폰을 다시 잡아도 시작하지 않는다. */
	UPROPERTY(VisibleInstanceOnly, Category = "Enemy|State")
	bool bHalted = false;
};
