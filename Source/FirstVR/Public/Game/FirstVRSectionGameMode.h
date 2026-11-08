#pragma once

#include "CoreMinimal.h"
#include "Game/FirstVRGameMode.h"
#include "FirstVRSectionGameMode.generated.h"

class AEncounterSection;

/**
 * 이동하며 구간을 돌파하는 레벨의 게임 모드(연구소). 실패·결과 화면·다시 시작은 부모 그대로다.
 * 시작하면 레벨의 구간 액터(AEncounterSection)를 순서대로 모아 첫 구간을 켠다. 플레이어가 켜진 구간의 설 자리에 오면
 * 그 구간의 웨이브 데이터를 물려받은 UWaveSpawner에 넣어 돌리고, 모두 막으면 체력을 채우고 문을 풀고 다음 구간을 켠다.
 * 웨이브 없는 구간은 도착하는 순간 끝난다. 마지막 구간까지 끝나면 승리.
 */
UCLASS()
class FIRSTVR_API AFirstVRSectionGameMode : public AFirstVRGameMode
{
	GENERATED_BODY()

public:
	/** 지금 차례인 구간(1부터). 시작 전이거나 다 끝났으면 0. */
	UFUNCTION(BlueprintPure, Category = "Encounter|Section")
	int32 GetCurrentSectionNumber() const { return Sections.IsValidIndex(CurrentSectionIndex) ? CurrentSectionIndex + 1 : 0; }

	UFUNCTION(BlueprintPure, Category = "Encounter|Section")
	int32 GetSectionCount() const { return Sections.Num(); }

	/** 지금 차례인 구간에 도착한 것으로 친다(Encounter.ArriveNext 시험용). */
	UFUNCTION(BlueprintCallable, Category = "Encounter|Section")
	void ForceArriveAtCurrentSection();

protected:
	virtual void StartEncounter() override;
	virtual void HandleAllWavesCleared() override;

private:
	UFUNCTION()
	void HandleSectionArrived(AEncounterSection* Section);

	void ActivateSection(int32 Index);
	void CompleteCurrentSection();
	void HealPlayer(float Amount) const;

	/** 순서대로 정렬한 레벨의 구간. */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Encounter|Section|State")
	TArray<TObjectPtr<AEncounterSection>> Sections;

	UPROPERTY(VisibleInstanceOnly, Category = "Encounter|Section|State")
	int32 CurrentSectionIndex = INDEX_NONE;
};
