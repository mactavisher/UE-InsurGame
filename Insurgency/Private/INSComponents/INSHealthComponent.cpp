// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSHealthComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UINSHealthComponent::UINSHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	MaximunHealth = 100;
	DefaultHealth = MaximunHealth;
	CurrentHealth = DefaultHealth;
	LowHealthPercentage = 0.5f;
	TimeBeforeHealthRestore = 5.f;
}


// Called when the game starts
void UINSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UINSHealthComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (OwnerCharacter)
	{
		if (CurrentHealth / MaximunHealth <= 0.4f)
		{
			OwnerCharacter->SetIsSuppressed(true);
		}
		if (CurrentHealth / MaximunHealth > 0.4f)
		{
			OwnerCharacter->SetIsSuppressed(false);
		}
	}
}

float UINSHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

void UINSHealthComponent::OnTakingDamage(float ReduceAmount, class AActor* DamageCauser, class AController* DamageInstigator)
{
	CurrentHealth -= FMath::CeilToInt(ReduceAmount);
	CurrentHealth = FMath::CeilToInt(FMath::Clamp<float>(CurrentHealth - ReduceAmount, 0.f, CurrentHealth));
	if (CurrentHealth <= 0)
	{
		OnCharacterShouldDie.Broadcast();
		GetWorld()->GetTimerManager().ClearTimer(HealthRestoreTimerHandle);
		DisableComponentTick();
	}
	GetWorld()->GetTimerManager().ClearTimer(HealthRestoreTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(HealthRestoreTimerHandle, this, &UINSHealthComponent::ReGenerateHealth, 0.1f, true, TimeBeforeHealthRestore);
}

void UINSHealthComponent::ReGenerateHealth()
{
	CurrentHealth = FMath::Clamp<float>(CurrentHealth += 1, CurrentHealth, MaximunHealth);
	if (CurrentHealth == MaximunHealth)
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthRestoreTimerHandle);
	}
}

float UINSHealthComponent::GetHealthPercentage()
{
	return CurrentHealth / MaximunHealth;
}

void UINSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UINSHealthComponent, MaximunHealth);
	DOREPLIFETIME(UINSHealthComponent, CurrentHealth);
}

bool UINSHealthComponent::CheckIsLowHealth()
{
	return GetHealthPercentage() <= LowHealthPercentage;
}

void UINSHealthComponent::EnableComponentTick()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UINSHealthComponent::DisableComponentTick()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void UINSHealthComponent::OnStopRestoreHealth()
{

}

