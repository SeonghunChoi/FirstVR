#include "Game/EnemySpawnPoint.h"

#include "Components/ArrowComponent.h"

AEnemySpawnPoint::AEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

#if WITH_EDITORONLY_DATA
	// 쿠킹한 게임에는 들어가지 않는 에디터 전용 컴포넌트다.
	Arrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (Arrow)
	{
		Arrow->SetupAttachment(Root);
		Arrow->ArrowColor = FColor::Red;
		Arrow->ArrowSize = 2.f;
		Arrow->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	}
#endif
}
