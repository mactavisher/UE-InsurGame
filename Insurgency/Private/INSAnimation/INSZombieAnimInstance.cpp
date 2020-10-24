// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSZombieAnimInstance.h"
#include "INSAI/AIZombie/INSZombie.h"
#include "INSComponents/INSCharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogINSZombieAnimation);
UINSZombieAnimInstance::UINSZombieAnimInstance(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bIsMoving = false;
	bIsFalling = false;
	bIsInAir = false;
	ZombieCurrentMoveMode = EZombieMoveMode::Shamble;
}

void UINSZombieAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ZombiePawnOwner = Cast<AINSZombie>(TryGetPawnOwner());
	if (ZombiePawnOwner)
	{
		ZombiePawnOwner = Cast<AINSZombie>(TryGetPawnOwner());
		ZombiePawnMovementComp = ZombiePawnOwner->GetINSCharacterMovement();
	}
	else
	{
		UE_LOG(LogINSZombieAnimation, Warning, TEXT("Failed to Initialize ZombiePawn!!!"));
	}
}

void UINSZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	UpdateIsMoving();
	UpdateIsFalling();
}

void UINSZombieAnimInstance::UpdateIsMoving()
{
	bIsMoving = ZombiePawnOwner && ZombiePawnMovementComp && ZombiePawnOwner->GetVelocity().Size2D() > 0.f;
}

void UINSZombieAnimInstance::UpdateIsFalling()
{
	bIsFalling = ZombiePawnMovementComp && ZombiePawnMovementComp->IsFalling();
}
