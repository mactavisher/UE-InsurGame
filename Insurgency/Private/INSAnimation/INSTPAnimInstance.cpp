// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSTPAnimInstance.h"

#ifndef UINSStaticAnimData
#include "INSAssets/INSStaticAnimData.h"
#endif
#ifndef UCharacterMovementComponent
#include "INSComponents/INSCharacterMovementComponent.h"
#endif
#ifndef AINSPlayerCharacter
#include "INSCharacter/INSPlayerCharacter.h"
#endif
UINSTPAnimInstance::UINSTPAnimInstance(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	JogPlayRate = 1.0f;
	WalkPlayRate = 1.0f;
	WalkToStopAlpha = 1.f;
	StandStopMoveAlpha = 1.f;
	SprintToWalkAlpha = 1.0f;
	bIsTurning = false;
	bCanEnterJog = false;
	TPShouldTurnLeft90 = false;
	CurrentStance = ECharacterStance::STAND;
	TPShouldTurnLeft90 = false;
	TPShouldTurnRight90 = bIsFalling;
	StandToJogAlpha = 0.f;
}

void UINSTPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	PlayWeaponBasePose();
	UpdateJogSpeed();
	UpdateCanEnterJogCondition();
	UpdateCanEnterSprint();
	//UpdateIsAiming();
	
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, bIsAiming ? TEXT("TP anim is aiming") : TEXT(" TP anim not aiming"));
}


void UINSTPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UINSTPAnimInstance::SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon)
{
	Super::SetCurrentWeaponAndAnimationData(NewWeapon);
	StandJogBlendSpace = CurrentWeaponAnimData->StandJogBlendSpace;
	StandWalkBlendSpace = CurrentWeaponAnimData->StandWalkBlendSpace;
}


void UINSTPAnimInstance::UpdateCanEnterJogCondition()
{
	bCanEnterJog = CharacterMovementComponent
		&& CharacterMovementComponent->GetLastUpdateVelocity().Size2D() > 0.f
		&& JogSpeed > 0.f;
}


void UINSTPAnimInstance::UpdateTurnConditions()
{
	if (GetOwningComponent() && CurrentViewMode == EViewMode::TPS)
	{
		const float DeltaYaw = OwnerPlayerCharacter->GetControlRotation().Yaw - GetOwningComponent()->GetForwardVector().Rotation().Yaw;
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
	if (CharacterMovementComponent)
	{
		const float CurrentSpeedValue = CharacterMovementComponent->GetLastUpdateVelocity().Size2D();
		const float MaxSpeed = CharacterMovementComponent->MaxWalkSpeed;
		JogSpeed = CurrentSpeedValue / MaxSpeed;
	}
	else
	{
		JogSpeed = 0.f;
	}
}

void UINSTPAnimInstance::UpdateCanEnterSprint()
{
	//bCanEnterSprint = bSprintPressed && CharacterMovementComponent->GetLastUpdateVelocity().Size2D() > 0.f;
}

void UINSTPAnimInstance::PlayReloadAnim(bool bIsDry)
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* SelectedReloadAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP:SelectedReloadAnim = bIsDry
		? CurrentWeaponAnimData->TPWeaponAltGripAnim.ReloadDryAnim.CharAnim
		: CurrentWeaponAnimData->TPWeaponAltGripAnim.ReloadAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP:SelectedReloadAnim = bIsDry
		? CurrentWeaponAnimData->TPWeaponForeGripAnim.ReloadDryAnim.CharAnim
		: CurrentWeaponAnimData->TPWeaponForeGripAnim.ReloadAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT:SelectedReloadAnim = bIsDry
		? CurrentWeaponAnimData->TPWeaponDefaultPoseAnim.ReloadDryAnim.CharAnim
		: CurrentWeaponAnimData->TPWeaponDefaultPoseAnim.ReloadAnim.CharAnim;
		break;
	default:SelectedReloadAnim = nullptr;
		break;
	}
	Montage_Play(SelectedReloadAnim);
}

void UINSTPAnimInstance::PlayWeaponBasePose()
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* SelectedBasePoseAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP:SelectedBasePoseAnim = CurrentWeaponAnimData->TPAltGripBasePose.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP:SelectedBasePoseAnim = CurrentWeaponAnimData->TPForeGripBasePose.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT:SelectedBasePoseAnim = CurrentWeaponAnimData->TPDefaultBasePose.CharAnim;
		break;
	default:SelectedBasePoseAnim = nullptr;
		break;
	}
	Montage_Play(SelectedBasePoseAnim);
}

void UINSTPAnimInstance::PlayFireAnim()
{
	if (!CheckValid())
	{
		return;
	}
	const uint8 RecoilAnimNum = CurrentWeaponAnimData->TPFireRecoilAnims.Num();
	const uint8 RandomIndex = FMath::RandHelper(RecoilAnimNum - 1);
	Montage_Play(CurrentWeaponAnimData->TPFireRecoilAnims[RandomIndex]);
	Montage_Play(CurrentWeaponAnimData->TPPulltriggerAnim.CharAnim);
}

void UINSTPAnimInstance::UpdateIsAiming()
{
	if (OwnerPlayerCharacter)
	{
		bIsAiming = OwnerPlayerCharacter->GetIsAiming();
	}
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

void UINSTPAnimInstance::SetIsAiming(bool IsAiming)
{
	Super::SetIsAiming(IsAiming);
}

void UINSTPAnimInstance::PlayWeaponStartEquipAnim()
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* SelectedEquipAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP:SelectedEquipAnim =
		CurrentWeaponAnimData->TPWeaponAltGripAnim.DeployAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP:SelectedEquipAnim =
		CurrentWeaponAnimData->TPWeaponForeGripAnim.DeployAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT:SelectedEquipAnim =
		CurrentWeaponAnimData->TPWeaponDefaultPoseAnim.DeployAnim.CharAnim;
		break;
	default:SelectedEquipAnim = nullptr;
		break;
	}
	Montage_Play(SelectedEquipAnim);
}

void UINSTPAnimInstance::UpdatePredictFallingToLandAlpha()
{
	// from this distance we start to update falling Alpha value ,any distance beyond this value will ignore and falling alpha is 0
	// means this character is full falling state and not prepared to land,with trace start hit frame , prepare to land and from where start 
	// to update falling alpha ,used for land and moving animation blending
	const float FallingCalMinDistance = 30.f;
	if (CharacterMovementComponent->IsFalling())
	{
		if (CurrentViewMode == EViewMode::TPS)
		{
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(OwnerPlayerCharacter);
			const FVector CharacterCurrentLocation = OwnerPlayerCharacter->GetActorLocation();

			const FVector TraceStartLocation = FVector(
				CharacterCurrentLocation.X,
				CharacterCurrentLocation.Y,
				CharacterCurrentLocation.Z - FMath::Abs(OwnerPlayerCharacter->BaseEyeHeight));
			const FVector TraceEndLocation = TraceStartLocation + FVector::DownVector * FallingCalMinDistance;

			FHitResult LandPredictHit(ForceInit);
			GetWorld()->LineTraceSingleByChannel(
				LandPredictHit,
				TraceStartLocation,
				TraceEndLocation,
				ECollisionChannel::ECC_Visibility,
				QueryParams);
			if (!LandPredictHit.bBlockingHit)
			{
				CustomNotIsFallingAlpha = 0.f;
			}
			else if (LandPredictHit.bBlockingHit)
			{
				const float Distance = FVector::Distance(TraceStartLocation, LandPredictHit.Location);
				CustomNotIsFallingAlpha = (1 - (Distance / FallingCalMinDistance));
				//if very close ,just set this to 1
				if (CustomNotIsFallingAlpha >= 1 - KINDA_SMALL_NUMBER)
				{
					CustomNotIsFallingAlpha = 1.0f;
				}
			}
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
			if (bShowDebugTrace)
			{
				DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Black, false, 0.1f);
			}
#endif
		}
	}
	else
	{
		CustomNotIsFallingAlpha = 1.f;
	}
}
