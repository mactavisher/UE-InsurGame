// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSHealthComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UINSHealthComponent::UINSHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	MaximunHealth = 100.f;
	DefaultHealth = MaximunHealth;
	CurrentHealth = DefaultHealth;
}


// Called when the game starts
void UINSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UINSHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

	// ...
}

float UINSHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

void UINSHealthComponent::ReduceHealth(float ReduceAmount, class AActor* DamageCauser, class AController* DamageInstigator)
{
	CurrentHealth -= ReduceAmount;
	FMath::Clamp<float>(CurrentHealth -= ReduceAmount, 0.f, 100);
	if (CurrentHealth <= 0.f)
	{
		OnCharacterShouldDie.Broadcast();
	}
	GetWorld()->GetTimerManager().ClearTimer(HealthRestoreTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(HealthRestoreTimerHandle, this, &UINSHealthComponent::ReGenerateHealth, 0.1f, true, TimeBeforeHealthRestore);
}

void UINSHealthComponent::ReGenerateHealth()
{

}

float UINSHealthComponent::GetHealthPercentage()
{
	return 0.f;
}

void UINSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

}

bool UINSHealthComponent::CheckIsLowHealth()
{
	return false;
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

