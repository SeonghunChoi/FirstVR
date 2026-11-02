#include "Weapon/Steps/TwoHandGripStep.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

UTwoHandGripStep::UTwoHandGripStep()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UTwoHandGripStep::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	USceneComponent* Root = Owner ? Owner->GetRootComponent() : nullptr;
	USceneComponent* Parent = AimParent.Get();
	if (!Root || !Parent || Root->GetAttachParent() != Parent || !SupportHand)
	{
		return;
	}

	const FTransform BaseWorld = AimBaseRelative * Parent->GetSocketTransform(Root->GetAttachSocketName());
	const FVector GripWorld = BaseWorld.TransformPosition(GripLocal);
	const FVector CurrentDir = BaseWorld.TransformPosition(SupportLocal) - GripWorld;
	const FVector TargetDir = SupportHand->GetComponentLocation() - GripWorld;
	if (CurrentDir.IsNearlyZero() || TargetDir.IsNearlyZero())
	{
		return;
	}

	const FQuat Delta = FQuat::FindBetweenNormals(CurrentDir.GetSafeNormal(), TargetDir.GetSafeNormal());
	const FVector NewLocation = GripWorld + Delta.RotateVector(BaseWorld.GetLocation() - GripWorld);
	Owner->SetActorLocationAndRotation(NewLocation, Delta * BaseWorld.GetRotation());
}

void UTwoHandGripStep::Execute(FFireContext& Context)
{
	const bool bTwoHanded = IsTwoHanded();
	const float Scale = bTwoHanded ? TwoHandRecoilScale : OneHandRecoilScale;
	Context.RecoilScale *= Scale;

	UE_LOG(LogWeapon, Log, TEXT("[%s] %s, recoil scale x%.2f"), *GetName(), bTwoHanded ? TEXT("Two hands") : TEXT("One hand"), Scale);
	Report(EStepResult::Success);
}

void UTwoHandGripStep::SetMainHand(USceneComponent* Hand)
{
	if (MainHand == Hand)
	{
		return;
	}

	MainHand = Hand;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Main %s (%s)"), *GetName(), Hand ? TEXT("held") : TEXT("released"), *GetNameSafe(Hand));
	UpdateTwoHandAim();
	UpdateSupportHold();
}

void UTwoHandGripStep::SetSupportHand(USceneComponent* Hand)
{
	if (SupportHand == Hand)
	{
		return;
	}

	SupportHand = Hand;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Support %s (%s)"), *GetName(), Hand ? TEXT("held") : TEXT("released"), *GetNameSafe(Hand));
	UpdateTwoHandAim();
	UpdateSupportHold();
}

void UTwoHandGripStep::UpdateTwoHandAim()
{
	if (IsTwoHanded() && !bAiming)
	{
		BeginTwoHandAim();
	}
	else if (!IsTwoHanded() && bAiming)
	{
		EndTwoHandAim();
	}
}

void UTwoHandGripStep::BeginTwoHandAim()
{
	AActor* Owner = GetOwner();
	USceneComponent* Root = Owner ? Owner->GetRootComponent() : nullptr;
	USceneComponent* Grip = FindGripComponent();
	USceneComponent* Support = FindSupportComponent();
	if (!Root || !Root->GetAttachParent() || !Grip || !Support)
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] Two-hand aim skipped (weapon not attached or grips missing)"), *GetName());
		return;
	}

	const FTransform& ActorTransform = Owner->GetActorTransform();
	GripLocal = ActorTransform.InverseTransformPosition(Grip->GetComponentLocation());
	SupportLocal = ActorTransform.InverseTransformPosition(Support->GetComponentLocation());
	AimParent = Root->GetAttachParent();
	AimBaseRelative = Root->GetRelativeTransform();

	bAiming = true;
	SetComponentTickEnabled(true);
	UE_LOG(LogWeapon, Log, TEXT("[%s] Two-hand aim on (parent %s)"), *GetName(), *GetNameSafe(AimParent.Get()));
}

void UTwoHandGripStep::EndTwoHandAim()
{
	bAiming = false;
	SetComponentTickEnabled(false);

	USceneComponent* Root = GetOwner() ? GetOwner()->GetRootComponent() : nullptr;
	const bool bRestore = Root && AimParent.IsValid() && Root->GetAttachParent() == AimParent.Get();
	if (bRestore)
	{
		Root->SetRelativeTransform(AimBaseRelative);
	}

	UE_LOG(LogWeapon, Log, TEXT("[%s] Two-hand aim off%s"), *GetName(), bRestore ? TEXT(", one-hand pose restored") : TEXT(""));
	AimParent.Reset();
}

void UTwoHandGripStep::UpdateSupportHold()
{
	const bool bSupportOnly = SupportHand && !MainHand;
	if (bSupportOnly && !bHeldBySupport)
	{
		AttachToSupportHand();
	}
	else if (!bSupportOnly && bHeldBySupport)
	{
		if (MainHand)
		{
			bHeldBySupport = false;
			UE_LOG(LogWeapon, Log, TEXT("[%s] Main hand took over"), *GetName());
		}
		else
		{
			DropFromSupportHand();
		}
	}
}

void UTwoHandGripStep::AttachToSupportHand()
{
	UPrimitiveComponent* Body = GetOwner() ? Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()) : nullptr;
	if (!Body || !SupportHand)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] Root is not a primitive component, cannot attach to support hand"), *GetName());
		return;
	}

	Body->SetSimulatePhysics(false);
	Body->AttachToComponent(SupportHand, FAttachmentTransformRules::KeepWorldTransform);
	bHeldBySupport = true;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Attached to support hand (%s)"), *GetName(), *GetNameSafe(SupportHand.Get()));
}

void UTwoHandGripStep::DropFromSupportHand()
{
	bHeldBySupport = false;

	UPrimitiveComponent* Body = GetOwner() ? Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()) : nullptr;
	if (!Body)
	{
		return;
	}

	Body->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Body->SetSimulatePhysics(true);
	UE_LOG(LogWeapon, Log, TEXT("[%s] Dropped from support hand"), *GetName());
}

USceneComponent* UTwoHandGripStep::FindGripComponent() const
{
	return FindNamedComponent(GripComponentName);
}

USceneComponent* UTwoHandGripStep::FindSupportComponent() const
{
	return FindNamedComponent(SupportComponentName);
}

USceneComponent* UTwoHandGripStep::FindNamedComponent(FName Name) const
{
	USceneComponent* Component = FindSceneComponentByName(GetOwner(), Name);
	if (!Component)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] No component named %s"), *GetName(), *Name.ToString());
	}
	return Component;
}
