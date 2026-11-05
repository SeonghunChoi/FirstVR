#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagazineDispenser.generated.h"

class AMagazine;
class UArrowComponent;

/**
 * 탄창 보급대. 자기 자리에 탄창 하나를 놓아 두고, 그 탄창을 가져가면 RespawnDelay 뒤 같은 자리에 새 탄창을 놓는다.
 * "가져갔다"는 탄창이 손이나 무기에 붙었거나, 무기에 끼워졌거나, 자리에서 TakeDistance보다 멀어졌거나, 사라진 것이다.
 * 어떤 탄창을 줄지는 배치마다 정한다. 받침대 모양은 블루프린트 자식에서 단다. 액터 위치가 받침 윗면이다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API AMagazineDispenser : public AActor
{
	GENERATED_BODY()

public:
	AMagazineDispenser();

	/** 지금 자리에 놓인 탄창. 가져갔거나 다시 놓기를 기다리는 중이면 None. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Dispenser")
	AMagazine* GetCurrentMagazine() const { return CurrentMagazine; }

protected:
	virtual void BeginPlay() override;

	/** 새 탄창을 놓았을 때. 소리·빛 같은 피드백은 블루프린트에서 단다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Dispenser")
	void OnMagazineDispensed(AMagazine* Magazine);

	/** 이 자리에서 줄 탄창. 비워 두면 아무것도 놓지 않는다. */
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Dispenser")
	TSubclassOf<AMagazine> MagazineClass;

	/** 가져간 뒤 새 탄창을 놓기까지 시간(초). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Dispenser", meta = (ClampMin = "0.0"))
	float RespawnDelay = 3.f;

	/** 놓인 탄창이 자리에서 이 거리(cm)보다 멀어지면 가져간 것으로 본다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Dispenser", meta = (ClampMin = "1.0"))
	float TakeDistance = 20.f;

	/** 받침 윗면에서 얼마나 위(cm)에 놓을지. 받침과 겹치지 않고 물리로 내려앉게 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Dispenser", meta = (ClampMin = "0.0"))
	float SpawnHeight = 10.f;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Dispenser")
	TObjectPtr<USceneComponent> Root;

#if WITH_EDITORONLY_DATA
	/** 에디터에서만 보이는 자리 표시. */
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Dispenser")
	TObjectPtr<UArrowComponent> Arrow;
#endif

private:
	/** 자리에 새 탄창을 놓는다. */
	void Dispense();

	/** 놓인 탄창을 가져갔는지 주기적으로 본다. */
	void CheckTaken();

	/** 지금 자리에 놓인 탄창. */
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Dispenser|State")
	TObjectPtr<AMagazine> CurrentMagazine;

	FTimerHandle CheckTimer;
	FTimerHandle RespawnTimer;
};
