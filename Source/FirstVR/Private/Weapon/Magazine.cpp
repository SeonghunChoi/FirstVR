#include "Weapon/Magazine.h"

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"
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

void AMagazine::HandleDropped()
{
	// 템플릿 잡기(BP_GrabComponent)는 OnDropped를 알린 뒤에도 놓기 처리(떼기, 물리 켜기)를 이어서 할 수 있다.
	// 같은 틱에 붙이면 그 처리가 다시 떼어 낼 수 있어서 한 틱 미룬다. 유니티로 치면 다음 프레임에 처리하는 코루틴.
	GetWorldTimerManager().SetTimerForNextTick(this, &AMagazine::FinishDrop);
}

void AMagazine::FinishDrop()
{
	if (IsInserted() || GetAttachParentActor())
	{
		// 이미 끼워졌거나 그사이 다시 손에 잡혔다.
		return;
	}

	if (UMagazineStep* Step = FindNearbyStep())
	{
		if (Step->InsertMagazine(this))
		{
			return;
		}
	}

	// 끼울 곳이 없으면 바닥으로 떨어진다. 끼워졌다 손으로 빼낸 탄창은 물리가 꺼져 있을 수 있어 여기서 켠다.
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Body->SetSimulatePhysics(true);
}

void AMagazine::HandleGrabbed()
{
	UMagazineStep* Step = InsertedInto.Get();
	if (!Step)
	{
		return;
	}

	// 끼워진 탄창의 잡기 컴포넌트를 다른 손이 잡으면 템플릿 잡기가 이미 손에 붙였다. 무기 쪽 상태만 맞춘다.
	InsertedInto.Reset();
	Step->ReleaseMagazine(this);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	UE_LOG(LogWeapon, Log, TEXT("[%s] Pulled out by hand (%d/%d left)"), *GetName(), Rounds, Capacity);
	OnInsertedChanged(false);
}

UMagazineStep* AMagazine::FindNearbyStep() const
{
	const FVector Top = GetTopLocation();
	UMagazineStep* Nearest = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	FString NearestName;

	// 무기는 몇 개뿐이라 모든 탄창 단계를 돌아본다. 유니티의 FindObjectsOfType<MagazineStep>()과 같다.
	for (TObjectIterator<UMagazineStep> It; It; ++It)
	{
		UMagazineStep* Step = *It;
		if (Step->GetWorld() != GetWorld() || !Step->HasBegunPlay() || Step->GetInsertedMagazine())
		{
			continue;
		}

		const USceneComponent* Well = Step->FindMagazineWell();
		if (!Well)
		{
			continue;
		}

		const float Distance = FVector::Dist(Top, Well->GetComponentLocation());
		if (Distance < NearestDistance && Step->CanAcceptMagazine(this))
		{
			Nearest = Step;
			NearestDistance = Distance;
			NearestName = Step->GetOwner()->GetName();
		}
	}

	if (!Nearest)
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Dropped, no empty weapon that fits"), *GetName());
		return nullptr;
	}

	if (NearestDistance > Nearest->GetInsertRadius())
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] Dropped, nearest well on %s is %.0f cm away (radius %.0f)"), *GetName(), *NearestName, NearestDistance, Nearest->GetInsertRadius());
		return nullptr;
	}

	return Nearest;
}

FVector AMagazine::GetTopLocation() const
{
	const USceneComponent* Top = FindSceneComponentByName(this, TopComponentName);
	return Top ? Top->GetComponentLocation() : GetActorLocation();
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
