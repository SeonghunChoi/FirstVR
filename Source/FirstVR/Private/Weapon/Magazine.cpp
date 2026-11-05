#include "Weapon/Magazine.h"

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include "TimerManager.h"
#include "Weapon/AmmoDataAsset.h"
#include "Weapon/Steps/MagazineStep.h"
#include "Weapon/WeaponTypes.h"

AMagazine::AMagazine()
{
	PrimaryActorTick.bCanEverTick = false;

	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	Body->InitBoxExtent(FVector(2.f, 1.5f, 6.f));
	Body->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Body->SetSimulatePhysics(true);
	RootComponent = Body;
}

void AMagazine::BeginPlay()
{
	Super::BeginPlay();
	Rounds = InitialRounds < 0 ? Capacity : FMath::Min(InitialRounds, Capacity);
}

UAmmoDataAsset* AMagazine::TakeRound()
{
	if (Rounds <= 0)
	{
		return nullptr;
	}

	--Rounds;
	return AmmoType;
}

void AMagazine::AttachToWell(UMagazineStep* Step, USceneComponent* Well)
{
	GetWorldTimerManager().ClearTimer(ClearTimer);
	InsertedInto = Step;

	// 무기에 붙은 동안은 무기 몸체와 겹쳐 있으므로 물리와 충돌을 끈다. 유니티로 치면 Rigidbody.isKinematic + Collider 끄기.
	Body->SetSimulatePhysics(false);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachToComponent(Well, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 지금은 몸통 중심이 Well에 있다. 윗면 표시(Top)가 Well에 오도록 몸통을 Top의 상대 트랜스폼의 역만큼 옮긴다.
	if (const USceneComponent* Top = FindSceneComponentByName(this, TopComponentName))
	{
		Body->SetRelativeTransform(Top->GetRelativeTransform().Inverse());
	}
	else
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] '%s' not found, snapping the body center"), *GetName(), *TopComponentName.ToString());
	}

	UE_LOG(LogWeapon, Log, TEXT("[%s] Inserted into %s (%s %d/%d)"), *GetName(), *GetNameSafe(Well ? Well->GetOwner() : nullptr), *GetNameSafe(AmmoType), Rounds, Capacity);
	OnInsertedChanged(true);
}

void AMagazine::DetachFromWell(bool bDrop)
{
	InsertedInto.Reset();
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (bDrop)
	{
		// 무기 몸체 안에서 물리가 켜지면 밀려 튕겨 나가므로, 떨어져 나갈 동안만 PhysicsBody(무기 몸체)를 무시한다.
		Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Body->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		Body->SetSimulatePhysics(true);
		GetWorldTimerManager().SetTimer(ClearTimer, this, &AMagazine::RestoreWeaponCollision, FMath::Max(EjectClearTime, 0.01f), false);
	}

	UE_LOG(LogWeapon, Log, TEXT("[%s] Ejected (%d/%d left)"), *GetName(), Rounds, Capacity);
	OnInsertedChanged(false);
}

void AMagazine::RestoreWeaponCollision()
{
	Body->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
}
