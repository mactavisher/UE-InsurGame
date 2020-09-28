// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSCharacterMovementComponent.h"

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
}

void UINSCharacterMovementComponent::StartCrouch()
{
	MaxWalkSpeed = BaseWalkSpeed * CrouchSpeedModifier;
	SpeedBeforeAim = MaxWalkSpeed;
}

void UINSCharacterMovementComponent::EndCrouch()
{
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
