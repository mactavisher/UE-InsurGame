// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSZombieAnimInstance.h"
#include "INSAI/AIZombie/INSZombie.h"
#include "INSComponents/INSCharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogINSZombieAnimation);

UINSZombieAnimInstance::UINSZombieAnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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
	if (ZombiePawnOwner && ZombiePawnMovementComp)
	{
		UpdateIsFalling();
		UpdateIsMovingHorizontal();
		EvaluateAnimCurves();
	}
}

void UINSZombieAnimInstance::EvaluateAnimCurves()
{
}

void UINSZombieAnimInstance::UpdateIsMoving()
{
	const float HorizontalSpeed = ZombiePawnOwner && ZombiePawnMovementComp ? 0.f : ZombiePawnMovementComp->GetLastUpdateVelocity().Size2D();
	bIsMoving = HorizontalSpeed > 0.f;
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	if (bIsMoving)
	{
		FString DebugMessage = FString("Zombie is moving");
		DebugMessage.Append(FString::SanitizeFloat(HorizontalSpeed));
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, DebugMessage);
	}
#endif
}

void UINSZombieAnimInstance::UpdateIsMovingHorizontal()
{
	if (ZombiePawnOwner && ZombiePawnMovementComp)
	{
		//bIsMovingHorizontal = !ZombiePawnMovementComp->IsFalling() && ZombiePawnMovementComp->GetLastUpdateVelocity().Size2D() > 0.f;
		bIsMovingHorizontal = !(ZombiePawnMovementComp->IsFalling()) && ZombiePawnMovementComp->GetLastUpdateVelocity().Size2D() > 0.f;
	}
}

void UINSZombieAnimInstance::UpdateIsFalling()
{
	bIsFalling = ZombiePawnMovementComp && ZombiePawnMovementComp->IsFalling();
}
