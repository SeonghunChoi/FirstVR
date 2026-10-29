// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/WeaponTypes.h"
#include "WeaponStepComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStepResult, EStepResult, Result);


UCLASS( Abstract )
class FIRSTVR_API UWeaponStepComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponStepComponent();

public:
	virtual void Execute(FFireContext& Context);
	
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Step")
	FOnStepResult OnStepResult;
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Weapon|Step")
	void Report(EStepResult Result);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Step")
	void OnFailedFeedback();
		
};
