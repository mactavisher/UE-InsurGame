// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSHealthComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

UINSHealthComponent::UINSHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	MaximunHealth = 100;
	DefaultHealth = MaximunHealth;
	CurrentHealth = DefaultHealth;
	LowHealthPercentage = 0.5f;
	TimeBeforeHealthRestore = 5.f;
}

void UINSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AINSCharacter>(GetOwner());
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

bool UINSHealthComponent::OnTakingDamage(float ReduceAmount, class AActor* DamageCauser, class AController* DamageInstigator)
{
	if (GetOwnerCharacter() && GetOwnerCharacter()->GetLocalRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthRestoreTimerHandle);
		CurrentHealth = FMath::CeilToInt(FMath::Clamp<float>(CurrentHealth - ReduceAmount, 0.f, CurrentHealth));
		if (CurrentHealth == 0)
		{
			OwnerCharacter->Die();
			DisableComponentTick();
		}
		else
		{
			const APlayerState* const PlayerState = GetOwnerCharacter()->GetController()->GetPlayerState<APlayerState>();

			if (PlayerState && !PlayerState->IsABot())
			{
				GetWorld()->GetTimerManager().SetTimer(HealthRestoreTimerHandle, this, &UINSHealthComponent::ReGenerateHealth, 0.1f, true, TimeBeforeHealthRestore);
			}
		}
	}
	return CurrentHealth == 0.f;
}

void UINSHealthComponent::ReGenerateHealth()
{
	if (GetOwnerCharacter() && GetOwnerCharacter()->GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp<float>(CurrentHealth += 1, CurrentHealth, MaximunHealth);
		if (CurrentHealth == MaximunHealth)
		{
			GetWorld()->GetTimerManager().ClearTimer(HealthRestoreTimerHandle);
		}
	}
}

float UINSHealthComponent::GetHealthPercentage()
{
	return CurrentHealth / MaximunHealth;
}

void UINSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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

