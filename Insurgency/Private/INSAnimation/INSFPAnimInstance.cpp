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
	bSighLocReCalculated = false;
	ADSAlpha = 0.f;
	ADSTime = 0.f;
	WeaponSwayLocation = FVector::ZeroVector;
	MaxWeaponSwayDelta = 6.5f;
	WeaponSwaySpeed = 12.f;
	MaxWeaponSwayDeltaAimingModifier = 0.6f;
	WeaponSwayLocationFactor = 0.5f;
	CurrentWeaponAnimData = nullptr;
	AdjustableAdsPoseRef = nullptr;
	OwnerPlayerController = nullptr;
}

void UINSFPAnimInstance::UpdateAdsAlpha(float DeltaSeconds)
{
	if (CurrentWeapon)
	{
		ADSAlpha = CurrentWeapon->GetWeaponADSAlpha();
	}
}


void UINSFPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!OwnerPlayerController)
	{
		return;
	}
	UpdateWeaponIkSwayLocationAndRotation(DeltaSeconds);
	UpdateAdsAlpha(DeltaSeconds);
	UpdateFiringHandsShift(DeltaSeconds);
	PlayWeaponBasePose();
	FPPlayMovingAnim();
	UpdateSight(DeltaSeconds);
	UpdateCanEnterSprint();
	FPPlayIdleAnim();
	UpdateAimHandsIKXLocation(DeltaSeconds);
}

void UINSFPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UINSFPAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	if (OwnerPlayerCharacter)
	{
		AController* Controller = OwnerPlayerCharacter->GetController();
		OwnerPlayerController = Cast<AINSPlayerController>(Controller);
		if (OwnerPlayerController)
		{
			LastRotation = OwnerPlayerController->GetControlRotation();
		}
	}
}

void UINSFPAnimInstance::UpdateSight(float DeltaTimeSeconds)
{
	if (bIsAiming && ADSAlpha == 1.f && !bSighLocReCalculated)
	{
		CalculateSight(DeltaAimSightTransform);
		bSighLocReCalculated = true;
	}
	//const float InterpSpeed = 1.f / ADSTime;
	if (!DeltaAimSightTransform.Equals(FTransform::Identity))
	{
		const FVector TargetSightLoc = DeltaAimSightTransform.GetLocation();
		const float TargetLocX = TargetSightLoc.X;
		const float TargetLocY = TargetSightLoc.Y;
		const float TargetLocZ = TargetSightLoc.Z;
		const float InterpSpeed = FMath::Abs(TargetLocZ / 0.05f);
		//SightLocWhenAiming.Z = FMath::Clamp<float>(SightLocWhenAiming.Z + InterpSpeed, SightLocWhenAiming.Z, TargetLocZ);
		SightLocWhenAiming.Z = FMath::FInterpTo(SightLocWhenAiming.Z, TargetLocZ, DeltaTimeSeconds, InterpSpeed);
		ADSHandIKEffector.Z = SightLocWhenAiming.Z;
		//ADSHandIKEffector.Y = -TargetLocY;
	}
	if (!bIsAiming)
	{
		SightLocWhenAiming.Z = FMath::Clamp<float>(SightLocWhenAiming.Z - 0.1f, 0.f, SightLocWhenAiming.Z);
		ADSHandIKEffector.Z = SightLocWhenAiming.Z;
		bSighLocReCalculated = false;
		DeltaAimSightTransform = FTransform::Identity;
	}
}

void UINSFPAnimInstance::UpdateAimHandsIKXLocation(float DeltaTimeSeconds)
{
	const float TargetValue = AimHandIKXLocationValue - BaseHandIKEffector.X;
	//const float InterpSpeed = FMath::Abs(TargetValue / (ADSTime * (1.f / DeltaTimeSeconds)));
	const float InterpSpeed = FMath::Abs(TargetValue * DeltaTimeSeconds / ADSTime);
	if (bIsAiming)
	{
		CurrentAimHandIKXLocationValue = FMath::Clamp<float>(CurrentAimHandIKXLocationValue - InterpSpeed, TargetValue, CurrentAimHandIKXLocationValue);
	}
	else
	{
		CurrentAimHandIKXLocationValue = FMath::Clamp<float>(CurrentAimHandIKXLocationValue + InterpSpeed, CurrentAimHandIKXLocationValue, 0.f);
	}
	ADSHandIKEffector.X = CurrentAimHandIKXLocationValue;
}

void UINSFPAnimInstance::UpdateWeaponIkSwayLocationAndRotation(float DeltaSeconds)
{
	if (OwnerPlayerController)
	{
		const FRotator CurrentRotation = OwnerPlayerController->GetControlRotation();
		WeaponIKSwayRotation = FMath::RInterpTo(WeaponIKSwayRotation, CurrentRotation - LastRotation, DeltaSeconds, WeaponSwaySpeed);
		WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(WeaponIKSwayRotation.Pitch, -MaxWeaponSwayDelta, MaxWeaponSwayDelta);
		WeaponIKSwayRotation.Roll = FMath::Clamp<float>(WeaponIKSwayRotation.Roll, -MaxWeaponSwayDelta, MaxWeaponSwayDelta);
		WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(WeaponIKSwayRotation.Yaw, -MaxWeaponSwayDelta, MaxWeaponSwayDelta);
		if (bIsAiming)
		{
			WeaponIKSwayRotation.Yaw = WeaponIKSwayRotation.Yaw * MaxWeaponSwayDeltaAimingModifier;
			WeaponIKSwayRotation.Roll = WeaponIKSwayRotation.Roll * MaxWeaponSwayDeltaAimingModifier;
			WeaponIKSwayRotation.Pitch = WeaponIKSwayRotation.Pitch * MaxWeaponSwayDeltaAimingModifier;
		}
		WeaponSwayLocation = FVector(WeaponIKSwayRotation.Pitch, WeaponIKSwayRotation.Yaw, WeaponIKSwayRotation.Pitch) * WeaponSwayLocationFactor;
		LastRotation = CurrentRotation;
	}
}

void UINSFPAnimInstance::FPStopIdleAnim()
{
	if (Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim))
	{
		Montage_Stop(0.15f, CurrentWeaponAnimData->FPIdleAnim);
	}
}

void UINSFPAnimInstance::FPStopMoveAnim()
{
	if (Montage_IsPlaying(CurrentWeaponAnimData->FPMoveAnim))
	{
		Montage_Stop(0.15f, CurrentWeaponAnimData->FPMoveAnim);
	}
}

void UINSFPAnimInstance::FPStopAimMoveAnim()
{
	if (Montage_IsPlaying(CurrentWeaponAnimData->FPAimMoveAnim))
	{
		Montage_Stop(0.15f, CurrentWeaponAnimData->FPAimMoveAnim);
	}
}

void UINSFPAnimInstance::FPPlayMovingAnim()
{
	if (!CheckValid())
	{
		return;
	}
	if (bIsMoving)
	{
		FPStopIdleAnim();
		if (bIsAiming)
		{
			FPStopMoveAnim();
			if (ADSAlpha >= 1.f)
			{
				if (!Montage_IsPlaying(CurrentWeaponAnimData->FPAimMoveAnim))
				{
					Montage_Play(CurrentWeaponAnimData->FPAimMoveAnim);
				}
			}
		}
		else
		{
			FPStopAimMoveAnim();
			if (!Montage_IsPlaying(CurrentWeaponAnimData->FPMoveAnim))
			{
				Montage_Play(CurrentWeaponAnimData->FPMoveAnim);
			}
		}
	}
	else
	{
		FPStopAimMoveAnim();
		FPStopMoveAnim();
	}
}


void UINSFPAnimInstance::FPPlayIdleAnim()
{
	if (!CheckValid())
	{
		return;
	}
	if (bIdleState)
	{
		if (!Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim))
		{
			Montage_Play(CurrentWeaponAnimData->FPIdleAnim);
		}
	}
	else
	{
		Montage_Stop(0.1f, CurrentWeaponAnimData->FPIdleAnim);
	}
}

void UINSFPAnimInstance::CalculateSight(FTransform& OutRelativeTransform)
{
	if (CurrentWeapon->IsSightAlignerExist())
	{
		FTransform CameraTrans = FTransform::Identity;
		Cast<AINSPlayerCharacter>(OwnerPlayerCharacter)->GetPlayerCameraSocketWorldTransform(CameraTrans);
		FTransform targetSightTransform;
		targetSightTransform.SetLocation(CameraTrans.GetLocation() + 10.f * CameraTrans.GetRotation().Vector());
		targetSightTransform.SetRotation(CameraTrans.GetRotation());
		//DrawDebugSphere(GetWorld(), targetSightTransform.GetLocation(), 1.f, 10, FColor::Red,false,10.f);
		const FTransform SightTrans = CurrentWeapon->GetSightsTransform();
		OutRelativeTransform = UKismetMathLibrary::MakeRelativeTransform(targetSightTransform, SightTrans);
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, OutRelativeTransform.GetLocation().ToString());
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

void UINSFPAnimInstance::SetIsAiming(bool IsAiming)
{
	Super::SetIsAiming(IsAiming);
	if (bIsAiming)
	{
	}
	else
	{
		ADSHandIKEffector = FVector(0.f, 0.f, 0.f);
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
	if (CurrentWeaponAnimData)
	{
		const uint8 AnimNum = CurrentWeaponAnimData->FPBoredAnims.Num();
		if (!bBoredState && AnimNum > 0)
		{
			for (uint8 i = 0; i < AnimNum; i++)
			{
				if (Montage_IsPlaying(CurrentWeaponAnimData->FPBoredAnims[i]))
				{
					Montage_Stop(0.2f, CurrentWeaponAnimData->FPBoredAnims[i]);
					break;
				}
			}
		}
	}
}

void UINSFPAnimInstance::PlayBoredAnim()
{
	Super::PlayBoredAnim();
	//if we have any active bored animation is playing,just leave it
	if (CurrentWeaponAnimData)
	{
		const uint8 AnimNum = CurrentWeaponAnimData->FPBoredAnims.Num();
		if (AnimNum > 0)
		{
			for (uint8 i = 0; i < AnimNum; i++)
			{
				if (Montage_IsPlaying(CurrentWeaponAnimData->FPBoredAnims[i]))
				{
					return;
				}
			}
		}
		const uint8 RandomAnimIdx = FMath::RandHelper(AnimNum);
		Montage_Play(CurrentWeaponAnimData->FPBoredAnims[RandomAnimIdx]);
	}
}

void UINSFPAnimInstance::SetCurrentWeapon(class AINSWeaponBase* NewWeapon)
{
	Super::SetCurrentWeapon(NewWeapon);
}

void UINSFPAnimInstance::SetCurrentWeaponAnimData(UINSStaticAnimData* NewAnimData)
{
	Super::SetCurrentWeaponAnimData(NewAnimData);
	if (CurrentWeaponAnimData)
	{
		AdjustableAdsPoseRef = CurrentWeaponAnimData->AdjustableAdsRefPose;
	}
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
	case EWeaponBasePoseType::ALTGRIP: SelectedPullTriggerAnim = CurrentWeaponAnimData->FPWeaponAltGripAnim.PullTriggerAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedPullTriggerAnim = CurrentWeaponAnimData->FPWeaponForeGripAnim.PullTriggerAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedPullTriggerAnim = CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.PullTriggerAnim.CharAnim;
		break;
	default: SelectedPullTriggerAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedPullTriggerAnim);
}

float UINSFPAnimInstance::PlayWeaponSwitchFireModeAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedSwitchFireModeAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedSwitchFireModeAnim = CurrentWeaponAnimData->FPWeaponAltGripAnim.SwitchFireModeAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedSwitchFireModeAnim = CurrentWeaponAnimData->FPWeaponForeGripAnim.SwitchFireModeAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedSwitchFireModeAnim = CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.SwitchFireModeAnim.CharAnim;
		break;
	default: SelectedSwitchFireModeAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedSwitchFireModeAnim);
}

void UINSFPAnimInstance::UpdateFiringHandsShift(float DeltaSeconds)
{
}

float UINSFPAnimInstance::PlayWeaponEquipAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedEquipAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedEquipAnim = CurrentWeaponAnimData->FPWeaponAltGripAnim.DeployAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedEquipAnim = CurrentWeaponAnimData->FPWeaponForeGripAnim.DeployAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedEquipAnim = CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.DeployAnim.CharAnim;
		break;
	default: SelectedEquipAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedEquipAnim);
}

float UINSFPAnimInstance::PlayWeaponUnEquipAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedUnEquipMontage = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedUnEquipMontage = CurrentWeaponAnimData->FPWeaponAltGripAnim.UnDeployAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedUnEquipMontage = CurrentWeaponAnimData->FPWeaponForeGripAnim.UnDeployAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedUnEquipMontage = CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.UnDeployAnim.CharAnim;
		break;
	default: SelectedUnEquipMontage = nullptr;
		break;
	}
	return Montage_Play(SelectedUnEquipMontage);
}

float UINSFPAnimInstance::PlayWeaponReloadAnim(bool bDryReload)
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedReloadAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedReloadAnim = bDryReload
		                                                        ? CurrentWeaponAnimData->FPWeaponAltGripAnim.ReloadDryAnim.CharAnim
		                                                        : CurrentWeaponAnimData->FPWeaponAltGripAnim.ReloadAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedReloadAnim = bDryReload
		                                                         ? CurrentWeaponAnimData->FPWeaponForeGripAnim.ReloadDryAnim.CharAnim
		                                                         : CurrentWeaponAnimData->FPWeaponForeGripAnim.ReloadAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedReloadAnim = bDryReload
		                                                        ? CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.ReloadDryAnim.CharAnim
		                                                        : CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.ReloadAnim.CharAnim;
		break;
	default: SelectedReloadAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedReloadAnim);
}
