#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magazine.generated.h"

class UAmmoDataAsset;
class UBoxComponent;
class UMagazineStep;

/**
 * 집는 탄창. 탄의 원본이다(어떤 탄이 몇 발). 무기의 UMagazineStep은 끼워진 탄창에서 한 발씩 꺼낸다.
 * 무기에 끼우고 빼는 판단은 UMagazineStep이 하고, 탄창은 붙고 떨어지는 몸의 처리(물리·충돌·붙이기)만 한다.
 * 모양과 잡기 컴포넌트는 블루프린트 자식에서 정한다. 끼울 때는 이름이 TopComponentName인 Scene Component(탄창 윗면)를
 * MagazineWell에 맞춘다. 루트(Body)는 물리로 움직여야 해서 원점을 옮길 수 없기 때문이다.
 */
UCLASS(Abstract, Blueprintable)
class FIRSTVR_API AMagazine : public AActor
{
	GENERATED_BODY()

public:
	AMagazine();

	/** 한 발을 꺼낸다. 비어 있으면 nullptr. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
	UAmmoDataAsset* TakeRound();

	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	int32 GetRounds() const { return Rounds; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	int32 GetCapacity() const { return Capacity; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	UAmmoDataAsset* GetAmmoType() const { return AmmoType; }

	/** 무기에 끼워져 있는지. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	bool IsInserted() const { return InsertedInto.IsValid(); }

	/** UMagazineStep이 받아들였을 때 부른다. 물리·충돌을 끄고 Well에 스냅해 붙는다. */
	void AttachToWell(UMagazineStep* Step, USceneComponent* Well);

	/** UMagazineStep이 내보낼 때 부른다. bDrop이면 물리를 켜 떨어뜨린다. */
	void DetachFromWell(bool bDrop);

protected:
	virtual void BeginPlay() override;

	/** 끼워지거나 빠질 때. 결합음 같은 피드백은 블루프린트에서 단다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Magazine")
	void OnInsertedChanged(bool bInserted);

	/** 몸통 충돌체이자 루트. 놓여 있을 때 물리로 떨어진다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Magazine")
	TObjectPtr<UBoxComponent> Body;

	/** 이 탄창에 든 탄 종류. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
	TObjectPtr<UAmmoDataAsset> AmmoType;

	/** 장탄수. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine", meta = (ClampMin = "1"))
	int32 Capacity = 15;

	/** 처음 든 탄 수. 0보다 작으면 가득 채운다. 배치마다 다르게 줄 수 있다. */
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Magazine")
	int32 InitialRounds = -1;

	/** MagazineWell에 맞출 자리(탄창 윗면)를 표시한 Scene Component 이름. 없으면 몸통 중심을 맞춘다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
	FName TopComponentName = TEXT("MagazineTop");

	/** 빠진 뒤 무기 몸체와 충돌을 무시하는 시간(초). 겹친 채 물리가 켜져 튕겨 나가지 않게 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine", meta = (ClampMin = "0.0"))
	float EjectClearTime = 0.5f;

private:
	void RestoreWeaponCollision();

	/** 남은 탄 수. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Magazine|State", meta = (AllowPrivateAccess = "true"))
	int32 Rounds = 0;

	/** 끼워진 무기의 탄창 단계. 빠져 있으면 비어 있다. */
	TWeakObjectPtr<UMagazineStep> InsertedInto;

	FTimerHandle ClearTimer;
};
