// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSFPAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#ifndef AINSWeaponBase
#include "INSItems/INSWeapons/INSWeaponBase.h"
#endif // !AINSWeaponBase
#include "DrawDebugHelpers.h"
#ifndef INSPlayerCharacter
#include "INSCharacter/INSPlayerCharacter.h"
#endif // !1
#ifndef INSPlayerCharacter
#include "INSCharacter/INSPlayerController.h"
#endif // !1
#ifndef INSPlayerCharacter
#include "INSComponents/INSCharacterMovementComponent.h"
#endif // !1
#ifndef UINSStaticAnimData
#include "INSAssets/INSStaticAnimData.h"
#endif // !1


UINSFPAnimInstance::UINSFPAnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MaxWeaponSwayPitch = 5.f;
	bSighLocReCalculated = false;
	ADSAlpha = 0.f;
	ADSTime = 0.f;
}

void UINSFPAnimInstance::UpdateAdsAlpha(float DeltaSeconds)
{
	if (CurrentWeapon == nullptr)
	{
		return;
	}
	ADSAlpha = CurrentWeapon->GetWeaponADSAlpha();
}

void UINSFPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (OwnerPlayerController)
	{
		return;
	}
	UpdateWeaponIkSwayRotation(DeltaSeconds);
	UpdateAdsAlpha(DeltaSeconds);
	UpdateFiringHandsShift(DeltaSeconds);
	PlayWeaponBasePose();
	FPPlayIdleOrMovingAnim();
	UpdateSight();
	UpdateCanEnterSprint();
}

void UINSFPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	if (OwnerPlayerCharacter)
	{
		AController* Controller = OwnerPlayerCharacter->GetController();
		OwnerPlayerController = OwnerPlayerController == nullptr ? nullptr : Cast<AINSPlayerController>(Controller);
	}
}

void UINSFPAnimInstance::UpdateWeaponIkSwayRotation(float deltaSeconds)
{
	if (OwnerPlayerController)
	{
		const float RotYaw = OwnerPlayerController->GetInputAxisValue(TEXT("Turn"));
		const float RotPitch = OwnerPlayerController->GetInputAxisValue(TEXT("LookUp"));
		if (RotYaw == 0.f)
		{
			if (WeaponIKSwayRotation.Yaw > 0.f)
			{
				WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(
					WeaponIKSwayRotation.Yaw - GetWorld()->GetDeltaSeconds() * WeaponSwayRecoverySpeed, 0.f,
					MaxWeaponSwayYaw);
			}
			if (WeaponIKSwayRotation.Yaw < 0.f)
			{
				WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(
					WeaponIKSwayRotation.Yaw + GetWorld()->GetDeltaSeconds() * WeaponSwayRecoverySpeed,
					-MaxWeaponSwayYaw, 0.f);
			}
		}
		else
		{
			WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(
				WeaponIKSwayRotation.Yaw + GetWorld()->GetDeltaSeconds() * RotYaw * WeaponSwayScale, -MaxWeaponSwayYaw,
				MaxWeaponSwayYaw);
		}
		if (RotPitch == 0.f)
		{
			if (WeaponIKSwayRotation.Pitch > 0.f)
			{
				WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(
					WeaponIKSwayRotation.Pitch - GetWorld()->GetDeltaSeconds() * WeaponSwayRecoverySpeed, 0.f,
					MaxWeaponSwayPitch);
			}
			if (WeaponIKSwayRotation.Pitch < 0.f)
			{
				WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(
					WeaponIKSwayRotation.Pitch + GetWorld()->GetDeltaSeconds() * WeaponSwayRecoverySpeed,
					-MaxWeaponSwayPitch, 0.f);
			}
		}
		else
		{
			WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(
				WeaponIKSwayRotation.Pitch - GetWorld()->GetDeltaSeconds() * RotPitch * WeaponSwayScale,
				-MaxWeaponSwayPitch, MaxWeaponSwayPitch);
		}
	}
}

void UINSFPAnimInstance::FPPlayIdleOrMovingAnim()
{
	if (!CheckValid())
	{
		return;
	}
	if (bIsMoving)
	{
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPAimIdleAnim))
		{
			Montage_Stop(0.1f, CurrentWeaponAnimData->FPAimIdleAnim);
		}
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim))
		{
			Montage_Stop(0.1f, CurrentWeaponAnimData->FPIdleAnim);
		}
		if (bIsAiming)
		{
			if (Montage_IsPlaying(CurrentWeaponAnimData->FPMoveAnim))
			{
				Montage_Stop(0.1f, CurrentWeaponAnimData->FPMoveAnim);
			}
			if (ADSAlpha >= 1.f && !Montage_IsPlaying(CurrentWeaponAnimData->FPAimMoveAnim))
			{
				Montage_Play(CurrentWeaponAnimData->FPAimMoveAnim);
			}
		}
		else
		{
			if (Montage_IsPlaying(CurrentWeaponAnimData->FPAimMoveAnim))
			{
				Montage_Stop(0.1f, CurrentWeaponAnimData->FPAimMoveAnim);
			}
			if (!Montage_IsPlaying(CurrentWeaponAnimData->FPMoveAnim))
			{
				Montage_Play(CurrentWeaponAnimData->FPMoveAnim);
			}
		}
	}
	else
	{
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPMoveAnim))
		{
			Montage_Stop(0.25f, CurrentWeaponAnimData->FPMoveAnim);
		}
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPAimMoveAnim))
		{
			Montage_Stop(0.25f, CurrentWeaponAnimData->FPAimMoveAnim);
		}
		if (bIsAiming)
		{
			if (Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim))
			{
				Montage_Stop(0.1f, CurrentWeaponAnimData->FPIdleAnim);
			}
		}
		else
		{
			if (Montage_IsPlaying(CurrentWeaponAnimData->FPMoveAnim))
			{
				Montage_Stop(0.1f, CurrentWeaponAnimData->FPMoveAnim);
			}
			if (!Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim))
			{
				Montage_Play(CurrentWeaponAnimData->FPIdleAnim);
			}
		}
	}
}


void UINSFPAnimInstance::UpdateSight()
{
	if (bIsAiming && ADSAlpha == 1.f && !bSighLocReCalculated)
	{
		CalculateSight(SightTransform);
		bSighLocReCalculated = true;
	}
	const float InterpSpeed = 1.f / ADSTime;
	if (!SightTransform.Equals(FTransform::Identity))
	{
		const FVector TargetSightLoc = SightTransform.GetLocation();
		const float TargetLocX = TargetSightLoc.X;
		const float TargetLocY = TargetSightLoc.Y;
		const float TargetLocZ = TargetSightLoc.Z;
		SightLocWhenAiming.Z = FMath::Clamp<float>(-(SightLocWhenAiming.Z + InterpSpeed), -TargetLocZ,
			-SightLocWhenAiming.Z);
	}
	if (!bIsAiming)
	{
		SightLocWhenAiming.Z = FMath::Clamp<float>(SightLocWhenAiming.Z + InterpSpeed, SightLocWhenAiming.Z, 0.f);
		bSighLocReCalculated = false;
		SightTransform = FTransform::Identity;
	}
}

float UINSFPAnimInstance::PlayWeaponStartEquipAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedEquipAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedEquipAnim = CurrentWeaponAnimData->FPWeaponAltGripAnim.DeployAnim.
		CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedEquipAnim = CurrentWeaponAnimData->FPWeaponForeGripAnim.DeployAnim.
		CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedEquipAnim = CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.DeployAnim.
		CharAnim;
		break;
	default: SelectedEquipAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedEquipAnim);
}

void UINSFPAnimInstance::CalculateSight(FTransform& OutRelativeTransform)
{
	if (CurrentWeapon->IsSightAlignerExist())
	{
		FTransform CameraTrans = FTransform::Identity;
		Cast<AINSPlayerCharacter>(OwnerPlayerCharacter)->GetPlayerCameraSocketWorldTransform(CameraTrans);
		const FTransform SightTrans = CurrentWeapon->GetSightsTransform();
		OutRelativeTransform = UKismetMathLibrary::MakeRelativeTransform(SightTrans, CameraTrans);
#if WITH_EDITOR
		//DrawDebugSphere(GetWorld(), CameraTrans.GetLocation(), 2, 10, FColor::Red, false, 0.1f);
		//DrawDebugSphere(GetWorld(), SightTrans.GetLocation(), 2, 10, FColor::Red, false, 0.1f);
		//GEngine->AddOnScreenDebugMessage(-1, -1.0f, FColor::Green, OutRelativeTransform.GetLocation().ToString());
#endif
	}
	else
	{
		OutRelativeTransform = FTransform::Identity;
	}
}

float UINSFPAnimInstance::PlayWeaponBasePose()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedBasePoseAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedBasePoseAnim = CurrentWeaponAnimData->FPWeaponAltGripAnim.BasePoseAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedBasePoseAnim = CurrentWeaponAnimData->FPWeaponForeGripAnim.BasePoseAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedBasePoseAnim = CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.BasePoseAnim.CharAnim;
		break;
	default: SelectedBasePoseAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedBasePoseAnim);
}

float UINSFPAnimInstance::PlayReloadAnim(bool bIsDry)
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedReloadAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP:SelectedReloadAnim = bIsDry
		? CurrentWeaponAnimData->FPWeaponAltGripAnim.ReloadDryAnim.CharAnim
		: CurrentWeaponAnimData->FPWeaponAltGripAnim.ReloadAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP:SelectedReloadAnim = bIsDry
		? CurrentWeaponAnimData->FPWeaponForeGripAnim.ReloadDryAnim.CharAnim
		: CurrentWeaponAnimData->FPWeaponForeGripAnim.ReloadAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT:SelectedReloadAnim = bIsDry
		? CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.ReloadDryAnim.CharAnim
		: CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.ReloadAnim.CharAnim;
		break;
	default:SelectedReloadAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedReloadAnim);
}

void UINSFPAnimInstance::SetIsAiming(bool IsAiming)
{
	Super::SetIsAiming(IsAiming);
	if (bIsAiming)
	{
		WeaponSwayScale = 2.f;
		MaxWeaponSwayPitch *= 0.2f;
		MaxWeaponSwayYaw *= 0.2f;
	}
	else
	{
		ADSHandIKEffector = FVector(0.f, 0.f, 0.f);
		WeaponSwayScale = 5.f;
		MaxWeaponSwayPitch = 5.f;
		MaxWeaponSwayYaw = 5.f;
	}
}

void UINSFPAnimInstance::SetIsSprinting(bool bIsSprintingNow)
{
}

void UINSFPAnimInstance::SetSprintPressed(bool NewSprintPressed)
{
	Super::SetIsSprinting(NewSprintPressed);
	if (bIsSprinting)
	{
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPAimIdleAnim))
		{
			Montage_Stop(0.1f, CurrentWeaponAnimData->FPAimIdleAnim);
		}
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim))
		{
			Montage_Stop(0.1f, CurrentWeaponAnimData->FPIdleAnim);
		}
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPMoveAnim))
		{
			Montage_Stop(0.1f, CurrentWeaponAnimData->FPMoveAnim);
		}
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPAimMoveAnim))
		{
			Montage_Stop(0.1f, CurrentWeaponAnimData->FPAimMoveAnim);
		}
		if (!Montage_IsPlaying(CurrentWeaponAnimData->FPSprintAnim))
		{
			Montage_Play(CurrentWeaponAnimData->FPSprintAnim);
		}
	}
	else
	{
		if (Montage_IsPlaying(CurrentWeaponAnimData->FPSprintAnim))
		{
			Montage_Stop(0.2f, CurrentWeaponAnimData->FPSprintAnim);
		}
	}
}

void UINSFPAnimInstance::SetIdleState(bool NewIdleState)
{
	Super::SetIdleState(NewIdleState);
}

void UINSFPAnimInstance::SetBoredState(bool NewBoredState)
{
	Super::SetBoredState(NewBoredState);
}

float UINSFPAnimInstance::PlaySwitchFireModeAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedSwitchFireModeAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedSwitchFireModeAnim =
		CurrentWeaponAnimData->FPWeaponAltGripAnim.SwitchFireModeAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedSwitchFireModeAnim =
		CurrentWeaponAnimData->FPWeaponForeGripAnim.SwitchFireModeAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedSwitchFireModeAnim = CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.
		SwitchFireModeAnim.CharAnim;
		break;
	default: SelectedSwitchFireModeAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedSwitchFireModeAnim);
}

void UINSFPAnimInstance::SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon)
{
	Super::SetCurrentWeaponAndAnimationData(NewWeapon);
	AdjustableAdsPoseRef = CurrentWeaponAnimData->AdjustableAdsRefPose;
	ADSTime = NewWeapon->AimTime;
}

float UINSFPAnimInstance::PlayFireAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	if (!bIsAiming)
	{
		const uint8 HandsRecoilAnimNum = CurrentWeaponAnimData->FPFireHandsRecoilAnims.Num();
		const uint8 RandomIndex = FMath::RandHelper(HandsRecoilAnimNum - 1);
		Montage_Play(CurrentWeaponAnimData->FPFireHandsRecoilAnims[RandomIndex]);
	}
	if (bIsAiming)
	{
		const uint8 HandsRecoilAnimNum = CurrentWeaponAnimData->FPAdsFireHandsMediumCalibers.Num();
		const uint8 RandomIndex = FMath::RandHelper(HandsRecoilAnimNum - 1);
		Montage_Play(CurrentWeaponAnimData->FPAdsFireHandsMediumCalibers[RandomIndex]);
	}
	UAnimMontage* SelectedPullTriggerAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedPullTriggerAnim =
		CurrentWeaponAnimData->FPWeaponAltGripAnim.PullTriggerAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedPullTriggerAnim =
		CurrentWeaponAnimData->FPWeaponForeGripAnim.PullTriggerAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedPullTriggerAnim = CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.
		PullTriggerAnim.CharAnim;
		break;
	default: SelectedPullTriggerAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedPullTriggerAnim);
}

void UINSFPAnimInstance::UpdateFiringHandsShift(float DeltaSeconds)
{
}
