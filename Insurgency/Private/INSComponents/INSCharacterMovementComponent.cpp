// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSCharacterMovementComponent.h"
#include "INSCharacter/INSPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogINSCharacterMovementComponent)
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
	IdleStateTime = 1.f;
	BoredStateTime = 10.f;
	bInIdleState = false;
	bInBoredState = false;
	IdleCheckFunction.bCanEverTick = true;
	IdleCheckFunction.SetTickFunctionEnable(true);
}

void UINSCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	if (CharacterOwner)
	{
		IdleCheckFunction.AddPrerequisite(CharacterOwner, CharacterOwner->PrimaryActorTick);
	}
	//note this will still set OwnerPlayerCharacter to null is class is not compatible
	INSCharacterOwner = CharacterOwner == nullptr ? nullptr : Cast<AINSPlayerCharacter>(CharacterOwner);
}

void UINSCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CheckCharacterIdleState(DeltaTime);
}


void UINSCharacterMovementComponent::Crouch(bool bClientSimulation /* = false */)
{
	Super::Crouch(bClientSimulation);
	MaxWalkSpeed = BaseWalkSpeed * CrouchSpeedModifier;
	SpeedBeforeAim = MaxWalkSpeed;
}

void UINSCharacterMovementComponent::UnCrouch(bool bClientSimulation /* = false */)
{
	Super::UnCrouch(bClientSimulation);
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
	if (!CharacterOwner)
	{
		UE_LOG(LogTemp, Log, TEXT("INScharacterMovementcomp ticking without characterowner"));
	}
	if (INSCharacterOwner && !INSCharacterOwner->GetIsDead())
	{
		if (FMath::Abs(GetLastUpdateVelocity().Size()) > 0)
		{
			AccumulatedIdleTime = 0.f;
			bInIdleState = false;
			bInBoredState = false;
			INSCharacterOwner->OnOutIdleState();
		}
		else
		{
			AccumulatedIdleTime += DeltaTime;
			if (!bInIdleState && AccumulatedIdleTime >= IdleStateTime)
			{
				bInIdleState = true;
				INSCharacterOwner->OnEnterIdleState();
			}
			if (!bInBoredState && AccumulatedIdleTime >= BoredStateTime)
			{
				bInBoredState = true;
				INSCharacterOwner->OnEnterBoredState();
			}
		}
	}
}
