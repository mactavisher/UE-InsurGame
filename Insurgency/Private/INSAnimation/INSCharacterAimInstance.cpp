// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSCharacterAimInstance.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSCharacter/INSPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "INSAssets/INSStaticAnimData.h"

DEFINE_LOG_CATEGORY(LogINSCharacterAimInstance);

UINSCharacterAimInstance::UINSCharacterAimInstance(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bWeaponAnimDelegateBindingFinished = false;
	bIsAiming = false;
	Pitch = 0.f;
	Yaw = 0.5f;
	Direction = 0.f;
	CustomNotIsFallingAlpha = 1.f;
	LeftHandIkAlpha = 1.f;
	RightHandIkAlpha = 1.f;
	WeaponIKSwayRotation = FRotator::ZeroRotator;
	bIsMoving = false;
	bStartJump = false;
	bIsCrouching = false;
	bCanEnterSprint = false;
	CurrentWeaponBaseType = EWeaponBasePoseType::FOREGRIP;
#if WITH_EDITORONLY_DATA
	bShowDebugTrace = true;
#endif
	bIdleState = false;
	bBoredState = false;
	AimHandIKXLocationValue = 0.f;
	CurrentAimHandIKXLocationValue = 0.f;
	BaseHandIKEffector = FVector::ZeroVector;
}

void UINSCharacterAimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
}

void UINSCharacterAimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	OwnerPlayerCharacter = Cast<AINSPlayerCharacter>(TryGetPawnOwner());
	if (OwnerPlayerCharacter)
	{
		CharacterMovementComponent = OwnerPlayerCharacter->GetCharacterMovement();
	}
}

void UINSCharacterAimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (OwnerPlayerCharacter && CharacterMovementComponent && !OwnerPlayerCharacter->GetIsDead())
	{
		UpdateDirection();
		UpdateHorizontalSpeed();
		UpdateVerticalSpeed();
		UpdatePitchAndYaw();
		UpdateIsMoving();
		UpdateSprintPlaySpeed();
		UpdateIsFalling();
		UpdateIsAiming();
		UpdateWeaponBasePoseType();
	}
}

void UINSCharacterAimInstance::UpdateHorizontalSpeed()
{
	HorizontalSpeed = CharacterMovementComponent->GetLastUpdateVelocity().Size2D();
}

void  UINSCharacterAimInstance::UpdateVerticalSpeed()
{
	VerticalSpeed = CharacterMovementComponent->GetLastUpdateVelocity().Z;
}

bool UINSCharacterAimInstance::CheckValid()
{
	if (OwnerPlayerCharacter == nullptr)
	{
		/*UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("Owner character not exist,can't play animations"));*/
		return false;
	}
	if (OwnerPlayerCharacter->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		//UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("Owner character runs on Dedicated server,can't play animations"));
		return false;

	}
	if (OwnerPlayerCharacter->GetIsDead())
	{
		//UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("Owner character is dead,can't play animations"));
		return false;
	}
	if (!CurrentWeapon)
	{
		UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("Missing Current Weapon Ref,invalid for playing any weapon anim"));
		return false;
	}
	if (!CurrentWeaponAnimData)
	{
		UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("Missing Current Weapon asstes Ref,invalid for playing any weapon anim"));
		return false;
	}
	return true;
}

bool UINSCharacterAimInstance::IsFPPlayingWeaponIdleAnim()
{
	return CurrentWeaponAnimData && Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim);
}

void UINSCharacterAimInstance::StopFPPlayingWeaponIdleAnim()
{
	if (CurrentWeapon && CurrentWeaponAnimData)
	{
		if (IsFPPlayingWeaponIdleAnim())
		{
			Montage_Stop(0.2f, CurrentWeaponAnimData->FPIdleAnim);
		}
	}
}

void UINSCharacterAimInstance::FPPlayMoveAnimation()
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* CurrentMoveMontage = bIsAiming ? CurrentWeaponAnimData->FPMoveAnim : CurrentWeaponAnimData->FPAimMoveAnim;
	UAnimMontage* AimMoveMontage = CurrentWeaponAnimData->FPMoveAnim;
	UAnimMontage* MoveMontage = CurrentWeaponAnimData->FPAimMoveAnim;
	const bool bIsPlaying1pMoveAnim = Montage_IsPlaying(CurrentMoveMontage);

	//if no montage player currently, just play it
	if (!bIsPlaying1pMoveAnim)
	{
		Montage_Play(CurrentMoveMontage);
		//UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Character %s Is Playing Reload animation,animation name:%s"), *CurrentMoveMontage->GetName());
	}
	if (bIsAiming && Montage_IsPlaying(MoveMontage))
	{
		Montage_Stop(0.2f, MoveMontage);
		Montage_Play(AimMoveMontage);
	}
	if (!bIsAiming && Montage_IsPlaying(AimMoveMontage))
	{
		Montage_Stop(0.2f, AimMoveMontage);
		Montage_Play(MoveMontage);
	}
}

void UINSCharacterAimInstance::StopFPPlayMoveAnimation()
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* MoveMontage = CurrentWeaponAnimData->FPIdleAnim;
	UAnimMontage* AimMoveMontage = CurrentWeaponAnimData->FPMoveAnim;
	const bool bIsFPPlayingMoveMontage = Montage_IsPlaying(MoveMontage);
	const bool bIsFPPlayingAimMoveMontage = Montage_IsPlaying(AimMoveMontage);
	if (bIsFPPlayingMoveMontage)
	{
		Montage_Stop(0.25f, MoveMontage);
	}
	if (bIsFPPlayingAimMoveMontage)
	{
		Montage_Stop(0.25f, AimMoveMontage);
	}
}

void UINSCharacterAimInstance::UpdateDirection()
{
	Direction = CalculateDirection(OwnerPlayerCharacter->GetVelocity(), OwnerPlayerCharacter->GetActorRotation());
}


void UINSCharacterAimInstance::UpdatePitchAndYaw()
{
	const FRotator CharacterRotation = OwnerPlayerCharacter->GetActorRotation();
	const FRotator ControlRotation = OwnerPlayerCharacter->GetBaseAimRotation();
	Pitch = ControlRotation.Pitch <= 90.f ? ControlRotation.Pitch / 90.f : (ControlRotation.Pitch - 360.f) / 90.f;
	//Pitch = UKismetMathLibrary::MapRangeClamped(ControlRotation.Pitch, -90.f, 360.f, -1.f, 1.f);
	if (ControlRotation.Yaw > 0.f)
	{
		Yaw = UKismetMathLibrary::MapRangeClamped(ControlRotation.Yaw, 0.f, 90.f, 0.5f, 0.f) * 1.067f;
	}
	if (ControlRotation.Yaw < 0.f)
	{
		Yaw = UKismetMathLibrary::MapRangeClamped(ControlRotation.Yaw, 0.f, -90.f, 0.5f, 1.f) * 1.067f;
	}
}

void UINSCharacterAimInstance::UpdateIsAiming()
{
	bIsAiming = OwnerPlayerCharacter && OwnerPlayerCharacter->GetIsAiming();
}

void UINSCharacterAimInstance::UpdateWeaponBasePoseType()
{
	if (CurrentWeapon)
	{
		CurrentWeaponBaseType = CurrentWeapon->GetCurrentWeaponBasePose();
	}
}

void UINSCharacterAimInstance::UpdateHandsIk()
{
	/*if (CurrentWeaponRef)
	{
		const FVector BaseHandsIkPostion = CurrentWeaponRef->GetBaseHandsIk();
		const FVector
	}*/
}


void UINSCharacterAimInstance::UpdateSprintPlaySpeed()
{
	SprintPlaySpeed = CharacterMovementComponent->GetLastUpdateVelocity().Size2D() / CharacterMovementComponent->MaxWalkSpeed;
}

void UINSCharacterAimInstance::UpdateIsMoving()
{
	bIsMoving = CharacterMovementComponent && HorizontalSpeed > 0.f;
}


void UINSCharacterAimInstance::UpdateIsFalling()
{
	bIsFalling = CharacterMovementComponent->IsFalling();
}


float UINSCharacterAimInstance::PlayAimAnim()
{
	bIsAiming = true;
	return 0.f;
}

float UINSCharacterAimInstance::PlayStopAimAnim()
{
	bIsAiming = false;
	return 0.f;
}

float UINSCharacterAimInstance::PlaySprintAnim()
{
	bIsSprinting = true;
	float Duration = 0.f;
	UAnimMontage* const SelectedSprintMontage = CurrentWeaponAnimData->FPSprintAnim;
	if (!Montage_IsPlaying(SelectedSprintMontage))
	{
		Duration = Montage_Play(SelectedSprintMontage);
	}
	return Duration;
}

float UINSCharacterAimInstance::StopPlaySprintAnim()
{
	bIsSprinting = false;
	float blendTime = 0.3f;
	UAnimMontage* const SelectedSprintMontage = CurrentWeaponAnimData->FPSprintAnim;
	if (Montage_IsPlaying(SelectedSprintMontage))
	{
		Montage_Stop(blendTime, SelectedSprintMontage);
	}
	return blendTime;
}

void UINSCharacterAimInstance::OnCharacterJustLanded()
{

}

void UINSCharacterAimInstance::SetIdleState(bool NewIdleState)
{
	bIdleState = NewIdleState;
}

void UINSCharacterAimInstance::SetBoredState(bool NewBoredState)
{
	bBoredState = NewBoredState;
	if (bBoredState)
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(BoredAnimPlayTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(BoredAnimPlayTimer, this, &UINSCharacterAimInstance::PlayBoredAnim, 15.0, true, 0.f);
		}
	}
	else 
	{
		GetWorld()->GetTimerManager().ClearTimer(BoredAnimPlayTimer);
	}
}

void UINSCharacterAimInstance::SetBaseHandsIkLocation(const FVector NewLocation)
{
	BaseHandIKEffector = NewLocation;
}

void UINSCharacterAimInstance::OnWeaponAnimDelegateBindingFinished()
{
	bWeaponAnimDelegateBindingFinished = true;
	PlayWeaponStartEquipAnim();
}

float UINSCharacterAimInstance::FPPlayWeaponIdleAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	float Duration = 0.f;
	if (!IsFPPlayingWeaponIdleAnim())
	{
		Duration = Montage_Play(CurrentWeaponAnimData->FPIdleAnim);
	}
	return Duration;
}

float UINSCharacterAimInstance::PlayWeaponIdleAnim()
{
	if (CurrentViewMode == EViewMode::FPS)
	{
		return FPPlayWeaponIdleAnim();
	}
	return 0.f;
}

void UINSCharacterAimInstance::SetWeaponBasePoseType(EWeaponBasePoseType NewBasePoseType)
{
	CurrentWeaponBaseType = NewBasePoseType;
}

void UINSCharacterAimInstance::SetIsSprinting(const bool NewSprintState)
{
	bIsSprinting = NewSprintState;
}

void UINSCharacterAimInstance::SetSprintPressed(bool NewSprintPressed)
{
	this->bSprintPressed = NewSprintPressed;
	bCanEnterSprint = bSprintPressed;
}

void UINSCharacterAimInstance::UpdateCanEnterSprint()
{
	//bCanEnterSprint = bSprintPressed && CharacterMovementComponent;
	//bIsSprinting = bCanEnterSprint;
}

void UINSCharacterAimInstance::SetIsAiming(bool IsAiming)
{
	bIsAiming = IsAiming;
}



void UINSCharacterAimInstance::PlayBoredAnim()
{
}

void UINSCharacterAimInstance::SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon)
{
	CurrentWeapon = NewWeapon;
	CurrentWeaponAnimData = CurrentWeapon == nullptr? nullptr: CurrentWeapon->GetWeaponAnimDataPtr();
}

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
void UINSCharacterAimInstance::AddScreenAminDebugMessage(const UAnimMontage* const Anim)
{

}
#endif