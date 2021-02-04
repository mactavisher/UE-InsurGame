// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSTPAnimInstance.h"

UINSTPAnimInstance::UINSTPAnimInstance(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	JogPlayRate = 1.0f;
	WalkPlayRate = 1.0f;
	WalkToStopAlpha = 1.f;
	StandStopMoveAlpha = 1.f;
	SprintToWalkAlpha = 1.0f;
	bIsTurning = false;
	TPShouldTurnLeft90 = false;
	CurrentStance = ECharacterStance::STAND;
	TPShouldTurnLeft90 = false;
	TPShouldTurnRight90 = bIsFalling;
}

void UINSTPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	UpdateCanEnterJogFromSprint();
	UpdateStandStopMoveAlpha();
}

void UINSTPAnimInstance::UpdateStandStopMoveAlpha()
{
	if (CharacterMovementComponent)
	{
		float CharacterCurrentSpeed = CharacterMovementComponent->GetLastUpdateVelocity().Size2D();
		float CharacterMaxMoveSpeed = CharacterMovementComponent->MaxWalkSpeed;
		StandStopMoveAlpha = FMath::Abs(1.f - CharacterCurrentSpeed / CharacterMaxMoveSpeed);
	}
}

void UINSTPAnimInstance::UpdateCanEnterJogFromSprint()
{
	bCanEnterJogFromSprint = !bSprintPressed;
}

void UINSTPAnimInstance::UpdateWalkToStopAlpha()
{

}

void UINSTPAnimInstance::UpdateTurnConditions()
{
	if (GetOwningComponent() && CurrentViewMode == EViewMode::TPS)
	{
		const float DeltaYaw = OwnerPlayerCharacter->GetControlRotation().Yaw - GetOwningComponent()->GetForwardVector().Rotation().Yaw;
		//UE_LOG(LogINSCharacterAimInstance, Log, TEXT("DeltaYaw value with mesh and control:%f"), DeltaYaw);
		if (DeltaYaw <= -80.f)
		{
			TPShouldTurnLeft90 = true;
		}
		if (DeltaYaw >= 80.f)
		{
			TPShouldTurnRight90 = true;
		}
		else
		{
			TPShouldTurnLeft90 = false;
			TPShouldTurnRight90 = false;
		}
	}
}


void UINSTPAnimInstance::UpdateJogSpeed()
{
	const float CurrentSpeedValue = CharacterMovementComponent->GetLastUpdateVelocity().Size2D();
	const float MaxSpeed = CharacterMovementComponent->MaxWalkSpeed;
	JogSpeed = CurrentSpeedValue / MaxSpeed;
}

void UINSTPAnimInstance::UpdateCanEnterSprint()
{
	//bCanEnterSprint = bSprintPressed && CharacterMovementComponent->GetLastUpdateVelocity().Size2D() > 0.f;
}

void UINSTPAnimInstance::UpdateEnterJogState()
{
	/*if (bIsMoving)
	{
		float CurrentSpeedVar = CharacterMovementComponent->GetLastUpdateVelocity().Size2D();
		if (CurrentSpeedVar > 10.f)
		{
			bCanEnterJog = true;
		}
		else
		{
			bCanEnterJog = false;
		}
	}
	else
	{
		bCanEnterJog = false;
	}*/
}

void UINSTPAnimInstance::UpdateSprintToWalkAlpha()
{
	if (OwnerPlayerCharacter->GetIsSprint())
	{
		const float CurrentSpeed = CharacterMovementComponent->GetLastUpdateVelocity().Size();
		const float MaxWalkSpeed = CharacterMovementComponent->GetMaxSpeed();
		SprintToWalkAlpha = CurrentSpeed / MaxWalkSpeed;
	}
	else
	{
		SprintToWalkAlpha = 1.f;
	}
}