#include "Weapon/TriggerInputComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponBase.h"

UTriggerInputComponent::UTriggerInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTriggerInputComponent::EnableForHand(bool bRightHand)
{
	DisableTrigger();

	UInputMappingContext* Context = bRightHand ? RightContext : LeftContext;
	UInputAction* Action = bRightHand ? RightFireAction : LeftFireAction;
	if (!Context || !Action)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] Input context or action is not set for %s hand"), *GetName(), bRightHand ? TEXT("right") : TEXT("left"));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	AActor* Owner = GetOwner();
	Owner->EnableInput(PlayerController);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(Owner->InputComponent);
	if (!Input)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] Owner has no EnhancedInputComponent"), *GetName());
		return;
	}
	BindingHandle = Input->BindAction(Action, ETriggerEvent::Started, this, &UTriggerInputComponent::HandleFire).GetHandle();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(Context, 0);
	}

	ActiveContext = Context;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Trigger enabled (%s hand)"), *GetName(), bRightHand ? TEXT("right") : TEXT("left"));
}

void UTriggerInputComponent::DisableTrigger()
{
	if (!ActiveContext)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(Owner->InputComponent))
	{
		Input->RemoveBindingByHandle(BindingHandle);
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(ActiveContext);
		}
		Owner->DisableInput(PlayerController);
	}

	ActiveContext = nullptr;
	BindingHandle = 0;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Trigger disabled"), *GetName());
}

USceneComponent* UTriggerInputComponent::FindGripComponent() const
{
	USceneComponent* Grip = FindSceneComponentByName(GetOwner(), GripComponentName);
	if (!Grip)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] No component named %s"), *GetName(), *GripComponentName.ToString());
	}
	return Grip;
}

void UTriggerInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisableTrigger();
	Super::EndPlay(EndPlayReason);
}

void UTriggerInputComponent::HandleFire()
{
	if (AWeaponBase* Weapon = Cast<AWeaponBase>(GetOwner()))
	{
		Weapon->RequestFire();
	}
}
