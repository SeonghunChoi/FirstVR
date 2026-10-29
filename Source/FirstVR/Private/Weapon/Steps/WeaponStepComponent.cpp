// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Steps/WeaponStepComponent.h"

// Sets default values for this component's properties
UWeaponStepComponent::UWeaponStepComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UWeaponStepComponent::Execute(FFireContext& Context)
{
	UE_LOG(LogWeapon, Error, TEXT("[%s] Execute() is not implemented"), *GetName());
	Report(EStepResult::Failed);
}

void UWeaponStepComponent::Report(EStepResult Result)
{
	UE_LOG(LogWeapon, Log, TEXT("[%s] %s"), *GetName(), *UEnum::GetValueAsString(Result));
	
	if (Result == EStepResult::Failed)
	{
		OnFailedFeedback();
	}
	
	OnStepResult.Broadcast(Result);
}

