#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyDataAsset.generated.h"

/** 다리 좌우. 다리가 아닌 본은 None이다. */
UENUM(BlueprintType)
enum class ELegSide : uint8
{
	None,
	Left,
	Right
};

/** 적 종류 하나의 설정. 맞은 대상과 부위를 따라가는 값만 둔다. 피해 계산은 UDamageReceiver가 한다. */
UCLASS(BlueprintType)
class FIRSTVR_API UEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 맞은 본의 피해 배율. BoneMultipliers에 없는 본은 1.0이다. */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetBoneMultiplier(FName BoneName) const;

	/** 맞은 본이 다리(LeftLegBones 또는 RightLegBones)인지. */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsLegBone(FName BoneName) const;

	/** 맞은 본이 어느 쪽 다리인지. 다리가 아니면 None. */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	ELegSide GetLegSide(FName BoneName) const;

	/** 최대 체력. 현재 체력은 UDamageReceiver가 들고 있다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	/** 방어력. 탄의 관통력이 이보다 모자라면 관통력 / 방어력 비율만큼만 피해가 들어간다. 0이면 방어 없음. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.0"))
	float Armor = 1.f;

	/** 본 이름별 피해 배율. 키는 피직스 에셋에 바디가 있는 본 이름이다. 1.0이 아닌 본만 적으면 된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Body")
	TMap<FName, float> BoneMultipliers;

	/** 왼쪽 다리로 셀 본 이름. 다리 본에 들어간 피해는 좌우를 합쳐 절뚝임 누적에 더해진다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Body")
	TArray<FName> LeftLegBones;

	/** 오른쪽 다리로 셀 본 이름. 절뚝임을 시작시킨 탄이 맞은 쪽이 다친 다리가 된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Body")
	TArray<FName> RightLegBones;

	/** 다리 누적 피해가 이 값 이상이면 절뚝인다. 최종 피해(방어력, 배율 적용 후) 기준이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|State", meta = (ClampMin = "0.0"))
	float LimpThreshold = 40.f;

	/** 사망 후 몸을 남겨 두는 시간. 지나면 액터를 없앤다. 0이면 계속 남긴다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|State", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float CorpseLifeSpan = 10.f;
};
