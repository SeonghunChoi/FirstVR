#include "Player/PlayerHealth.h"

#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogPlayer);

UPlayerHealth::UPlayerHealth()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerHealth::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	// ApplyDamage → AActor::TakeDamage → OnTakeAnyDamage. 적의 공격이 이 방법으로 들어온다.
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UPlayerHealth::HandleTakeAnyDamage);

	UE_LOG(LogPlayer, Log, TEXT("[%s] Health %.0f/%.0f"), *GetOwner()->GetName(), CurrentHealth, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, 0.f);
}

void UPlayerHealth::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// 죽은 뒤에도 적이 계속 때리므로, 무시되는 피해는 로그도 남기지 않는다.
	if (bDead)
	{
		return;
	}

	UE_LOG(LogPlayer, Log, TEXT("[%s] Damaged by %s"), *GetOwner()->GetName(), *GetNameSafe(DamageCauser));
	TakeHealthDamage(Damage);
}

void UPlayerHealth::TakeHealthDamage(float Damage)
{
	if (bDead || Damage <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Max(0.f, CurrentHealth - Damage);
	UE_LOG(LogPlayer, Log, TEXT("[%s] Damage %.0f, health %.0f/%.0f"), *GetOwner()->GetName(), Damage, CurrentHealth, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, Damage);

	if (CurrentHealth <= 0.f)
	{
		bDead = true;
		UE_LOG(LogPlayer, Log, TEXT("[%s] Player died"), *GetOwner()->GetName());
		OnDied.Broadcast();
	}
}

static FAutoConsoleCommandWithWorldAndArgs PlayerDamageCommand(
	TEXT("Player.Damage"),
	TEXT("Applies damage to the player pawn through ApplyDamage. Usage: Player.Damage [Amount] (default 20)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const float Amount = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 20.f;
		APlayerController* Player = World ? World->GetFirstPlayerController() : nullptr;
		if (APawn* Pawn = Player ? Player->GetPawn() : nullptr)
		{
			UGameplayStatics::ApplyDamage(Pawn, Amount, nullptr, nullptr, UDamageType::StaticClass());
		}
	}));
