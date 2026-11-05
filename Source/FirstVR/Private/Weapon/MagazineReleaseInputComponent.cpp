#include "Weapon/MagazineReleaseInputComponent.h"

#include "Weapon/Magazine.h"
#include "Weapon/Steps/MagazineStep.h"
#include "Weapon/WeaponTypes.h"

void UMagazineReleaseInputComponent::BeginPlay()
{
	Super::BeginPlay();

	MagazineStep = GetOwner()->FindComponentByClass<UMagazineStep>();
	if (!MagazineStep)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] Owner has no MagazineStep"), *GetName());
	}
}

void UMagazineReleaseInputComponent::HandleAction()
{
	if (!MagazineStep)
	{
		return;
	}

	// 탄창이 없을 때 누른 것은 오류가 아니다. 시험할 때 알 수 있게 로그만 남긴다.
	if (!MagazineStep->EjectMagazine())
	{
		UE_LOG(LogWeapon, Log, TEXT("[%s] No magazine to eject"), *GetName());
	}
}
