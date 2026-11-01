#include "Weapon/WeaponInputComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponTypes.h"

UWeaponInputComponent::UWeaponInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponInputComponent::EnableForHand(bool bRightHand)
{
	DisableForHand();

	UInputMappingContext* Context = bRightHand ? RightContext : LeftContext;
	UInputAction* Action = bRightHand ? RightAction : LeftAction;
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
	BindingHandle = Input->BindAction(Action, ETriggerEvent::Started, this, &UWeaponInputComponent::HandleAction).GetHandle();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(Context, 0);
	}

	ActiveContext = Context;
	ActiveHand = bRightHand ? EControllerHand::Right : EControllerHand::Left;
	UE_LOG(LogWeapon, Log, TEXT("[%s] Input enabled (%s hand)"), *GetName(), bRightHand ? TEXT("right") : TEXT("left"));
}

void UWeaponInputComponent::DisableForHand()
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

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(ActiveContext);
		}
	}

	ActiveContext = nullptr;
	BindingHandle = 0;

	// 같은 무기의 다른 입력 컴포넌트가 아직 켜져 있으면 액터 입력을 남겨 둔다.
	const bool bActorInputDisabled = PlayerController && !IsAnyWeaponInputEnabled();
	if (bActorInputDisabled)
	{
		Owner->DisableInput(PlayerController);
	}
	UE_LOG(LogWeapon, Log, TEXT("[%s] Input disabled%s"), *GetName(), bActorInputDisabled ? TEXT(", actor input off") : TEXT(""));
}

bool UWeaponInputComponent::IsAnyWeaponInputEnabled() const
{
	TArray<UWeaponInputComponent*> Inputs;
	GetOwner()->GetComponents(Inputs);
	for (const UWeaponInputComponent* Input : Inputs)
	{
		if (Input->IsHandInputEnabled())
		{
			return true;
		}
	}
	return false;
}

USceneComponent* UWeaponInputComponent::FindGripComponent() const
{
	USceneComponent* Grip = FindSceneComponentByName(GetOwner(), GripComponentName);
	if (!Grip)
	{
		UE_LOG(LogWeapon, Error, TEXT("[%s] No component named %s"), *GetName(), *GripComponentName.ToString());
	}
	return Grip;
}

void UWeaponInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisableForHand();
	Super::EndPlay(EndPlayReason);
}
