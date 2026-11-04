#include "Enemy/AnimNotify_EnemyStrike.h"

#include "Components/SkeletalMeshComponent.h"
#include "Enemy/EnemyAttack.h"

void UAnimNotify_EnemyStrike::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 애니메이션 에디터 미리보기의 메시에는 UEnemyAttack이 없어서 아무 일도 하지 않는다.
	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (UEnemyAttack* Attack = Owner ? Owner->FindComponentByClass<UEnemyAttack>() : nullptr)
	{
		Attack->Strike(StrikeBone, Radius);
	}
}

FString UAnimNotify_EnemyStrike::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Strike %s"), *StrikeBone.ToString());
}
