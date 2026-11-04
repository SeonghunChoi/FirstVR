#include "Weapon/LaserSightComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Weapon/WeaponProjectile.h"
#include "Weapon/WeaponTypes.h"

ULaserSightComponent::ULaserSightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// 손(모션 컨트롤러)과 두 손 조준(TG_PostPhysics)이 무기를 옮긴 뒤에 빔을 맞춘다.
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void ULaserSightComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	Muzzle = FindSceneComponentByName(Owner, MuzzleComponentName);
	if (!Muzzle)
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] Laser found no component named %s"), *GetName(), *MuzzleComponentName.ToString());
		return;
	}
	if (!BeamSystem)
	{
		UE_LOG(LogWeapon, Warning, TEXT("[%s] Laser has no BeamSystem"), *GetName());
		return;
	}

	// 유니티에서 AddComponent로 런타임에 붙이는 것과 같다. 빔은 Muzzle을 따라 움직이고, 꺼진 채로 시작한다.
	Beam = NewObject<UNiagaraComponent>(Owner, TEXT("LaserBeam"));
	Beam->SetAsset(BeamSystem);
	Beam->SetAutoActivate(false);
	Beam->SetupAttachment(Muzzle);
	Beam->RegisterComponent();
}

void ULaserSightComponent::SetLaserEnabled(bool bEnabled)
{
	if (bLaserEnabled == bEnabled)
	{
		return;
	}

	bLaserEnabled = bEnabled;
	SetComponentTickEnabled(bEnabled && Beam);
	if (Beam)
	{
		if (bEnabled)
		{
			// 켜는 첫 프레임부터 제자리에 그리도록 위치를 먼저 넘긴다.
			UpdateBeam();
			Beam->Activate(true);
		}
		else
		{
			Beam->DeactivateImmediate();
		}
	}
	UE_LOG(LogWeapon, Log, TEXT("[%s] Laser %s"), *GetName(), bEnabled ? TEXT("on") : TEXT("off"));
}

void ULaserSightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateBeam();
}

void ULaserSightComponent::UpdateBeam()
{
	if (!Muzzle || !Beam)
	{
		return;
	}

	const FVector Start = Muzzle->GetComponentLocation();
	FVector End = Start + Muzzle->GetForwardVector() * MaxDistance;

	// 탄이 맞는 대상(월드, 적 메시)만 찾는다. Pawn(적 캡슐, 플레이어 몸)은 탄도 지나가므로 찾지 않는다.
	FCollisionObjectQueryParams TargetTypes(ECC_WorldStatic);
	TargetTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
	TargetTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(LaserSight), false, GetOwner());

	// 날아가는 탄에 빔이 걸리지 않게, 탄을 맞히면 무시 목록에 넣고 다시 찾는다.
	constexpr int32 MaxAttempts = 4;
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		FHitResult Hit;
		if (!GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, TargetTypes, Params))
		{
			break;
		}
		if (Cast<AWeaponProjectile>(Hit.GetActor()))
		{
			Params.AddIgnoredActor(Hit.GetActor());
			continue;
		}
		End = Hit.ImpactPoint;
		break;
	}

	Beam->SetVariablePosition(BeamStartParameter, Start);
	Beam->SetVariablePosition(BeamEndParameter, End);
}

USceneComponent* ULaserSightComponent::FindGripComponent() const
{
	return FindSceneComponentByName(GetOwner(), GripComponentName);
}
