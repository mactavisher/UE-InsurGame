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
		if (ADSAlpha == 1.f && CurrentWeapon && CurrentViewMode == EViewMode::FPS && ADSHandIKEffector.IsZero())
		{
			if (CurrentWeapon->IsSightAlignerExist())
			{
				FTransform CameraTrans = FTransform::Identity;
				Cast<AINSPlayerCharacter>(OwnerPlayerCharacter)->GetPlayerCameraSocketWorldTransform(CameraTrans);
				FTransform SightTrans = CurrentWeapon->GetSightsTransform();
				DrawDebugSphere(GetWorld(), CameraTrans.GetLocation(), 2, 10, FColor::Red, false, 0.1f);
				DrawDebugSphere(GetWorld(), SightTrans.GetLocation(), 2, 10, FColor::Red, false, 0.1f);
				const FTransform DeltaTrans = UKismetMathLibrary::MakeRelativeTransform(SightTrans, CameraTrans);
				GEngine->AddOnScreenDebugMessage(-1, -1.0f, FColor::Green, DeltaTrans.ToString());
				ADSHandIKEffector = FVector(0.f, 0.f, -DeltaTrans.GetLocation().Z);
			}
			/*if (ADSHandIKEffector.IsZero())
			{
				FTransform CameraTrans = Cast<AINSPlayerCharacter>(OwnerPlayerCharacter)->GetPlayerCameraTransform();
				FTransform WeaponSightTrans = CurrentWeaponRef->GetSightsTransform();
				FVector RelLoc = UKismetMathLibrary::MakeRelativeTransform(CameraTrans, WeaponSightTrans).GetLocation();
				ADSHandIKEffector = FVector(20.f, RelLoc.Y, +RelLoc.Z);
				CurrentHandIKEffector = ADSHandIKEffector;
			}
			GEngine->AddOnScreenDebugMessage(-1, -1.0f, FColor::Green, ADSHandIKEffector.ToString());

			CurrentHandIKEffector.X = FMath::Clamp<float>(CurrentHandIKEffector.X + DeltaTimeSeconds * 0.1f,CurrentHandIKEffector.X,ADSHandIKEffector.X);
			CurrentHandIKEffector.Y = FMath::Clamp<float>(CurrentHandIKEffector.Y + DeltaTimeSeconds * 0.1f, CurrentHandIKEffector.Y, ADSHandIKEffector.Y);
			CurrentHandIKEffector.Z = FMath::Clamp<float>(CurrentHandIKEffector.Z + DeltaTimeSeconds * 0.1f, CurrentHandIKEffector.Z, ADSHandIKEffector.Z);
			*/
		}
	}
	else
	{
		ADSAlpha = ADSAlpha - InterpSpeed * DeltaSeconds;
		if (ADSAlpha <= 0.f)
		{
			ADSAlpha = 0.f;
		}
		/*
		CurrentHandIKEffector = FVector::ZeroVector;
		ADSHandIKEffector = FVector(ForceInit);
		CurrentHandIKEffector.X = FMath::Clamp<float>(CurrentHandIKEffector.X - DeltaTimeSeconds * 0.1f, 0.f, CurrentHandIKEffector.X);
		CurrentHandIKEffector.Y = FMath::Clamp<float>(CurrentHandIKEffector.Y - DeltaTimeSeconds * 0.1f, 0.f, CurrentHandIKEffector.Y);
		CurrentHandIKEffector.Z = FMath::Clamp<float>(CurrentHandIKEffector.Z - DeltaTimeSeconds * 0.1f, 0.f, CurrentHandIKEffector.Z);
		*/
	}
}

void UINSFPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	UpdateAdsAlpha(DeltaSeconds);
	UpdateFiringHandsShift(DeltaSeconds);
	if (!OwnerPlayerController)
	{
		OwnerPlayerController = Cast<AINSPlayerController>(OwnerPlayerCharacter->GetController());
	}
	if (CurrentWeapon)
	{
		PlayWeaponBasePose(CurrentWeapon->GetIsWeaponHasForeGrip());
		if (bIsAiming && CurrentWeaponAnimData &&
			Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim))
		{
			Montage_Stop(0.2f, CurrentWeaponAnimData->FPIdleAnim);
		}
	}
	if (CurrentViewMode == EViewMode::FPS)
	{
		if (CharacterMovementComponent->GetLastUpdateVelocity().Size() >= 0.5f)
		{
			StopFPPlayingWeaponIdleAnim();
			FPPlayMoveAnimation();
		}
		if (CharacterMovementComponent->GetLastUpdateVelocity().Size() <= 0.5f)
		{
			StopFPPlayMoveAnimation();
			PlayWeaponIdleAnim();
		}
	}
}

void UINSFPAnimInstance::UpdateFiringHandsShift(float DeltaSeconds)
{

}

