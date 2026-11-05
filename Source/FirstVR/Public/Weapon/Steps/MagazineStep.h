#pragma once

#include "CoreMinimal.h"
#include "Weapon/Steps/WeaponStepComponent.h"
#include "MagazineStep.generated.h"

class AMagazine;

/**
 * 탄창 단계. 무기의 MagazineWell에 끼워진 탄창(AMagazine)에서 한 발을 꺼내 Context.Ammo에 기록한다.
 * 탄창이 없거나 비어 있으면 Failed. 탄의 원본은 탄창이고, 이 단계는 지금 끼워진 탄창만 가리킨다.
 * 어떤 탄창이 맞는지(AllowedMagazines), 끼우기·빼기 판단도 이 단계가 한다.
 */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class FIRSTVR_API UMagazineStep : public UWeaponStepComponent
{
	GENERATED_BODY()

public:
	virtual void Execute(FFireContext& Context) override;

	/** 이 무기에 맞는 탄창인지(AllowedMagazines). 이미 끼워진 탄창이 있어도 종류만 본다. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	bool CanAcceptMagazine(const AMagazine* Magazine) const;

	/** 탄창을 끼운다. 맞지 않거나, 이미 끼워져 있거나, MagazineWell이 없으면 false. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
	bool InsertMagazine(AMagazine* Magazine);

	/** 끼워진 탄창을 빼서 떨어뜨린다. 빼낸 탄창(없으면 nullptr)을 돌려준다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
	AMagazine* EjectMagazine();

	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	AMagazine* GetInsertedMagazine() const { return InsertedMagazine; }

	/** 이름이 MagazineWellName인 Scene Component. 탄창이 붙는 자리다. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	USceneComponent* FindMagazineWell() const;

	float GetInsertRadius() const { return InsertRadius; }

protected:
	virtual void BeginPlay() override;

	/** 끼워진 탄창이 바뀔 때(빠지면 None). 결합음 같은 피드백은 블루프린트에서 단다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Magazine")
	void OnMagazineChanged(AMagazine* Magazine);

	/** 이 무기에 맞는 탄창 클래스. 자식 클래스도 맞는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
	TArray<TSubclassOf<AMagazine>> AllowedMagazines;

	/** 탄창이 붙는 Scene Component 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
	FName MagazineWellName = TEXT("MagazineWell");

	/** 탄창을 놓았을 때 MagazineWell까지 이 거리(cm) 안이면 끼운다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine", meta = (ClampMin = "0.0"))
	float InsertRadius = 15.f;

	/** 배치별 시작 탄창. BeginPlay에서 스폰해 끼운다. 비워 두면 빈 무기로 시작한다. */
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Magazine")
	TSubclassOf<AMagazine> InitialMagazine;

private:
	/** 지금 끼워진 탄창. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Magazine|State", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AMagazine> InsertedMagazine;
};
