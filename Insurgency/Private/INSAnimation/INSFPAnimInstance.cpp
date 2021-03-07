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




UINSFPAnimInstance::UINSFPAnimInstance(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	MaxWeaponSwayPitch = 5.f;
	bSighLocReCalculated = false;
}

void UINSFPAnimInstance::UpdateAdsAlpha(float DeltaSeconds)
{
	if (CurrentWeapon == nullptr)
	{
		return;
	}
	const float WeaponAimTime = CurrentWeapon->GetWeaponAimTime();
	const float InterpSpeed = 1.f / WeaponAimTime;
	if (bIsAiming)
	{
		ADSAlpha = ADSAlpha + InterpSpeed * DeltaSeconds;
		if (ADSAlpha >= 1.f)
		{
			ADSAlpha = 1.0f;
		}
		ADSAlpha = FMath::Clamp<float>(ADSAlpha + InterpSpeed * DeltaSeconds
			, ADSAlpha
			, 1.f);
	}
	else
	{
		ADSAlpha = FMath::Clamp<float>(ADSAlpha - InterpSpeed * DeltaSeconds
			, 0.f
			, ADSAlpha);
	}
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
	FPCheckAndPlayIdleAnim();
	FPCheckAndPlayMovingAnim();
	UpdateSight();
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
				WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(WeaponIKSwayRotation.Yaw -= GetWorld()->GetDeltaSeconds() * WeaponSwayRecoverySpeed, 0.f, MaxWeaponSwayYaw);
			}
			if (WeaponIKSwayRotation.Yaw < 0.f)
			{
				WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(WeaponIKSwayRotation.Yaw += GetWorld()->GetDeltaSeconds() * WeaponSwayRecoverySpeed, -MaxWeaponSwayYaw, 0.f);
			}
		}
		else
		{
			WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(WeaponIKSwayRotation.Yaw += GetWorld()->GetDeltaSeconds() * RotYaw * WeaponSwayScale, -MaxWeaponSwayYaw, MaxWeaponSwayYaw);
		}
		if (RotPitch == 0.f)
		{
			if (WeaponIKSwayRotation.Pitch > 0.f)
			{
				WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(WeaponIKSwayRotation.Pitch -= GetWorld()->GetDeltaSeconds() * WeaponSwayRecoverySpeed, 0.f, MaxWeaponSwayPitch);
			}
			if (WeaponIKSwayRotation.Pitch < 0.f)
			{
				WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(WeaponIKSwayRotation.Pitch += GetWorld()->GetDeltaSeconds() * WeaponSwayRecoverySpeed, -MaxWeaponSwayPitch, 0.f);
			}
		}
		else
		{
			WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(WeaponIKSwayRotation.Pitch -= GetWorld()->GetDeltaSeconds() * RotPitch * WeaponSwayScale, -MaxWeaponSwayPitch, MaxWeaponSwayPitch);
		}
	}
}

void UINSFPAnimInstance::FPCheckAndPlayIdleAnim()
{
	if (!CheckValid())
	{
		return;
	}
	if (!bIsMoving)
	{
		// kill any moving montages first
		Montage_Stop(0.25f, CurrentWeaponAnimData->FPAimMoveAnim);
		Montage_Stop(0.25f, CurrentWeaponAnimData->FPMoveAnim);
		if (bIsAiming)
		{
			Montage_Stop(0.25f, CurrentWeaponAnimData->FPMoveAnim);
			Montage_Stop(0.25f, CurrentWeaponAnimData->FPIdleAnim);
		}
	}
}

void UINSFPAnimInstance::FPCheckAndPlayMovingAnim()
{
	if (!CheckValid())
	{
		return;
	}
	if (CurrentWeapon)
	{
		if (bIsAiming)
		{
			if (bIsMoving && !Montage_IsPlaying(CurrentWeaponAnimData->FPAimMoveAnim))
			{
				Montage_Play(CurrentWeaponAnimData->FPAimMoveAnim);
			}
		}
		else
		{
			if (bIsMoving && !Montage_IsPlaying(CurrentWeaponAnimData->FPMoveAnim))
			{
				Montage_Play(CurrentWeaponAnimData->FPMoveAnim);
			}
		}
	}
}


void UINSFPAnimInstance::UpdateSight()
{
	if (bIsAiming && ADSAlpha == 1.f&&!bSighLocReCalculated)
	{
		CalculateSight(SightTransform);
		bSighLocReCalculated = true;
	}
	const float interpSpeed = 1 / ADSTime;
	if (!SightTransform.Equals(FTransform::Identity))
	{
		const FVector TargetSightLoc = SightTransform.GetLocation();
		const float TargetLocX = TargetSightLoc.X;
		const float TargetLocY = TargetSightLoc.Y;
		const float TargetLocZ = TargetSightLoc.Z;
		SightLocWhenAiming.Z = FMath::Clamp<float>(-(SightLocWhenAiming.Z + interpSpeed), -TargetLocZ, -SightLocWhenAiming.Z);
	}
	if(!bIsAiming)
	{
		SightLocWhenAiming.Z = FMath::Clamp<float>(SightLocWhenAiming.Z + interpSpeed, SightLocWhenAiming.Z,0.f);
		bSighLocReCalculated = false;
		SightTransform = FTransform::Identity;
	}
}

void UINSFPAnimInstance::PlayWeaponStartEquipAnim()
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* SelectedEquipAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP:SelectedEquipAnim =
		CurrentWeaponAnimData->FPWeaponAltGripAnim.DeployAnim.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP:SelectedEquipAnim =
		CurrentWeaponAnimData->FPWeaponForeGripAnim.DeployAnim.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT:SelectedEquipAnim =
		CurrentWeaponAnimData->FPWeaponDefaultPoseAnim.DeployAnim.CharAnim;
		break;
	default:SelectedEquipAnim = nullptr;
		break;
	}
	Montage_Play(SelectedEquipAnim);
}

void UINSFPAnimInstance::CalculateSight(FTransform& OutRelativeTransform)
{
	if (CurrentWeapon->IsSightAlignerExist())
	{
		FTransform CameraTrans = FTransform::Identity;
		Cast<AINSPlayerCharacter>(OwnerPlayerCharacter)->GetPlayerCameraSocketWorldTransform(CameraTrans);
		FTransform SightTrans = CurrentWeapon->GetSightsTransform();
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

void UINSFPAnimInstance::SetupAimIdleAnim(const bool NewAimingCondition)
{
	if (bIsAiming)
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(AimIdleAnimTimer))
		{
			const float WeaponADSTime = CurrentWeapon->GetWeaponAimTime();
			GetWorld()->GetTimerManager().SetTimer(AimIdleAnimTimer, this, &UINSFPAnimInstance::PlayAimIdleAnim, 1.f, true, WeaponADSTime);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(AimIdleAnimTimer);
	}
}

void UINSFPAnimInstance::PlayWeaponBasePose()
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* SelectedBasePoseAnim = nullptr;
	switch (CurrentWeaponBaseType)
	{
	case EWeaponBasePoseType::ALTGRIP:SelectedBasePoseAnim = CurrentWeaponAnimData->FPAltGripBasePose.CharAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP:SelectedBasePoseAnim = CurrentWeaponAnimData->FPForeGripBasePose.CharAnim;
		break;
	case EWeaponBasePoseType::DEFAULT:SelectedBasePoseAnim = CurrentWeaponAnimData->FPDefaultBasePose.CharAnim;
		break;
	default:SelectedBasePoseAnim = nullptr;
		break;
	}
	Montage_Play(SelectedBasePoseAnim);
}

void UINSFPAnimInstance::PlayReloadAnim(bool bIsDry)
{
	if (!CheckValid())
	{
		return;
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
	Montage_Play(SelectedReloadAnim);
}

void UINSFPAnimInstance::PlayAimIdleAnim()
{
	if (!Montage_IsPlaying(CurrentWeaponAnimData->FPAimIdleAnim))
	{
		Montage_Play(CurrentWeaponAnimData->FPAimIdleAnim);
	}
}

void UINSFPAnimInstance::SetIsAiming(bool IsAiming)
{
	Super::SetIsAiming(IsAiming);
	if (bIsAiming)
	{
		WeaponSwayScale = 2.f;
		MaxWeaponSwayPitch *= 0.2f;
		MaxWeaponSwayYaw *= 0.2f;
		//disable idle animation for now
		Montage_Stop(0.1f, CurrentWeaponAnimData->FPIdleAnim);
		Montage_Stop(0.1f, CurrentWeaponAnimData->FPMoveAnim);
	}
	else
	{
		ADSHandIKEffector = FVector(0.f, 0.f, 0.f);
		WeaponSwayScale = 5.f;
		MaxWeaponSwayPitch = 5.f;
		MaxWeaponSwayYaw = 5.f;
	}
	SetupAimIdleAnim(bIsAiming);
}

void UINSFPAnimInstance::SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon)
{
	Super::SetCurrentWeaponAndAnimationData(NewWeapon);
	AdjustableAdsPoseRef = CurrentWeaponAnimData->AdjustableAdsRefPose;
	ADSTime = NewWeapon->AimTime;
}

void UINSFPAnimInstance::PlayFireAnim()
{
	if (!CheckValid())
	{
		return;
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
	Montage_Play(CurrentWeaponAnimData->FPPulltriggerAnim.CharAnim);
}

void UINSFPAnimInstance::UpdateFiringHandsShift(float DeltaSeconds)
{

}

