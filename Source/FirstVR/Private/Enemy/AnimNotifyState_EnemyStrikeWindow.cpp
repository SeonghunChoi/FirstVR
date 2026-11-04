#include "Enemy/AnimNotifyState_EnemyStrikeWindow.h"

#include "Components/SkeletalMeshComponent.h"
#include "Enemy/EnemyAttack.h"

namespace
{
	// 애니메이션 에디터 미리보기의 메시에는 UEnemyAttack이 없어서 아무 일도 하지 않는다.
	UEnemyAttack* FindAttackOnMesh(const USkeletalMeshComponent* MeshComp)
	{
		const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
		return Owner ? Owner->FindComponentByClass<UEnemyAttack>() : nullptr;
	}
}

void UAnimNotifyState_EnemyStrikeWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (UEnemyAttack* Attack = FindAttackOnMesh(MeshComp))
	{
		Attack->BeginStrikeWindow(StrikeBone, Radius);
	}
}

void UAnimNotifyState_EnemyStrikeWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (UEnemyAttack* Attack = FindAttackOnMesh(MeshComp))
	{
		Attack->UpdateStrikeWindow();
	}
}

void UAnimNotifyState_EnemyStrikeWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (UEnemyAttack* Attack = FindAttackOnMesh(MeshComp))
	{
		Attack->EndStrikeWindow();
	}
}

FString UAnimNotifyState_EnemyStrikeWindow::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Strike Window %s"), *StrikeBone.ToString());
}
