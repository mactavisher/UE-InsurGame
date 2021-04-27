// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSCharacterMovementComponent.h"
#include "INSCharacter/INSPlayerCharacter.h"

UINSCharacterMovementComponent::UINSCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	BaseWalkSpeed = 300.f;
	MaxWalkSpeed = BaseWalkSpeed;
	SpeedBeforeAim = MaxWalkSpeed;
	SprintSpeedModifier = 2.0f;
	CrouchSpeedModifier = 0.8f;
	ProneSpeedModifier = 0.1f;
	AimSpeedModifier = 0.3f;
	MaxAcceleration = 500.f;
	AccumulatedIdleTime = 0.f;
	IdleStateTime = 10.f;
	bInIdleState = false;
}

void UINSCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	//note this will still set OwnerPlayerCharacter to null is class is not compatible
	INSCharacterOwner = CharacterOwner == nullptr ? nullptr : Cast<AINSPlayerCharacter>(OwnerPlayerCharacter);
}

void UINSCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	{
		CheckCharacterIdleState(DeltaTime);
	}
}

void UINSCharacterMovementComponent::StartCrouch()
{
	MaxWalkSpeed = BaseWalkSpeed * CrouchSpeedModifier;
	SpeedBeforeAim = MaxWalkSpeed;
	bWantsToCrouch = true;
}

void UINSCharacterMovementComponent::EndCrouch()
{
	bWantsToCrouch = false;
	MaxWalkSpeed = BaseWalkSpeed;
	SpeedBeforeAim = MaxWalkSpeed;
}

void UINSCharacterMovementComponent::StartSprint()
{
	MaxWalkSpeed = BaseWalkSpeed * SprintSpeedModifier;
	SpeedBeforeAim = MaxWalkSpeed;
}

void UINSCharacterMovementComponent::EndSprint()
{
	MaxWalkSpeed = BaseWalkSpeed;
	SpeedBeforeAim = MaxWalkSpeed;
}

void UINSCharacterMovementComponent::StartProne()
{
	MaxWalkSpeed = BaseWalkSpeed * ProneSpeedModifier;
	SpeedBeforeAim = MaxWalkSpeed;
}

void UINSCharacterMovementComponent::EndProne()
{
	MaxWalkSpeed = BaseWalkSpeed;
	SpeedBeforeAim = MaxWalkSpeed;
}

void UINSCharacterMovementComponent::StartAim()
{
	MaxWalkSpeed = SpeedBeforeAim * AimSpeedModifier;
}

void UINSCharacterMovementComponent::EndAim()
{
	MaxWalkSpeed = SpeedBeforeAim;
}

void UINSCharacterMovementComponent::CheckCharacterIdleState(const float DeltaTime)
{
	if (GetLastUpdateVelocity().Size() <= 0)
	{
		AccumulatedIdleTime += DeltaTime;
		if (bInIdleState && INSCharacterOwner)
		{
			if (!INSCharacterOwner->GetIsCharacterDead())
			{
				if (AccumulatedIdleTime >= IdleStateTime)
				{
					bInIdleState = true;
					INSCharacterOwner->OnEnterIdleState();
				}
				if (AccumulatedIdleTime >= BoredStateTime)
				{
					bInBoredState = true;
					INSCharacterOwner->OnEnterBoredState();
				}
			}
		}
	}
	else
	{
		if (!INSCharacterOwner->GetIsCharacterDead())
		{
			AccumulatedIdleTime = 0.f;
			bInIdleState = false;
			bInBoredState = false;
			INSCharacterOwner->OnOutIdleState();
		}
	}
}
