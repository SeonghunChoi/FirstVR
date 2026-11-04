#include "Player/PlayerBody.h"

#include "Camera/CameraComponent.h"
#include "Engine/CollisionProfile.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayer, Log, All);

UPlayerBody::UPlayerBody()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 카메라가 HMD 자세를 받은 뒤에 따라간다.
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	// 폰이 돌아도 캡슐은 서 있다.
	SetUsingAbsoluteRotation(true);
	SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	// 스폰할 때는 충돌을 꺼 둔다. 켜 두면 폰 원점(바닥)에 중심을 둔 캡슐이 바닥에 박힌 것으로 보여
	// 스폰 위치 보정이 폰을 캡슐 반 높이만큼 위로 밀어 올린다. BeginPlay에서 자리를 잡은 뒤 켠다.
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
	InitCapsuleSize(BodyRadius, 85.f);
}

void UPlayerBody::BeginPlay()
{
	Super::BeginPlay();

	Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		UE_LOG(LogPlayer, Warning, TEXT("[%s] PlayerBody found no camera"), *GetOwner()->GetName());
		return;
	}

	FollowHead();
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	UE_LOG(LogPlayer, Log, TEXT("[%s] Player body following %s (radius %.0f, height %.0f)"), *GetOwner()->GetName(), *Camera->GetName(), GetScaledCapsuleRadius(), GetScaledCapsuleHalfHeight() * 2.f);
}

void UPlayerBody::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FollowHead();
}

void UPlayerBody::FollowHead()
{
	if (!Camera)
	{
		return;
	}

	const FVector Head = Camera->GetComponentLocation();

	// 폰 위치는 바닥이 아닐 수 있다(PlayerStart 높이에 뜬 채 스폰된다). 머리 아래로 바닥을 찾는다.
	float FloorZ = GetOwner()->GetActorLocation().Z;
	FHitResult FloorHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerBodyFloor), false, GetOwner());
	// 레벨 바닥은 움직일 수 있는(BlockAllDynamic) 메시일 수도 있어서 WorldDynamic도 찾는다.
	FCollisionObjectQueryParams FloorTypes(ECC_WorldStatic);
	FloorTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
	if (GetWorld()->LineTraceSingleByObjectType(FloorHit, Head, Head - FVector(0.f, 0.f, 400.f), FloorTypes, Params))
	{
		FloorZ = FloorHit.ImpactPoint.Z;
	}
	const float HalfHeight = FMath::Max((Head.Z + HeadTopMargin - FloorZ) * 0.5f, BodyRadius);

	SetCapsuleSize(BodyRadius, HalfHeight, false);
	SetWorldLocation(FVector(Head.X, Head.Y, FloorZ + HalfHeight));
}

float UPlayerBody::GetDistanceToSurface(const FVector& Point) const
{
	const FVector Center = GetComponentLocation();
	const FVector Axis = GetUpVector() * GetScaledCapsuleHalfHeight_WithoutHemisphere();
	return FMath::PointDistToSegment(Point, Center - Axis, Center + Axis) - GetScaledCapsuleRadius();
}

float UPlayerBody::GetDistanceToSurface(const FVector& Start, const FVector& End) const
{
	const FVector Center = GetComponentLocation();
	const FVector Axis = GetUpVector() * GetScaledCapsuleHalfHeight_WithoutHemisphere();
	FVector OnSegment;
	FVector OnAxis;
	FMath::SegmentDistToSegmentSafe(Start, End, Center - Axis, Center + Axis, OnSegment, OnAxis);
	return FVector::Dist(OnSegment, OnAxis) - GetScaledCapsuleRadius();
}
