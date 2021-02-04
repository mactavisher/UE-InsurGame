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
	WeaponIKRootOffSetEffector = FVector::ZeroVector;
	WeaponIKLeftHandOffSetEffector = FVector::ZeroVector;
	WeaponIKRightHandOffSetEffector = FVector::ZeroVector;
	WeaponIKSwayRotation = FRotator::ZeroRotator;
	bIsMoving = false;
	bStartJump = false;
	bIsCrouching = false;
	WeaponIKSwayRotationAlpha = 0.f;
	CurrentViewMode = EViewMode::FPS;
	MaxWeaponSwayPitch = 5.f;
#if WITH_EDITORONLY_DATA
	bShowDebugTrace = true;
#endif
}

void UINSCharacterAimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerPlayerCharacter = Cast<AINSPlayerCharacter>(TryGetPawnOwner());
	if (OwnerPlayerCharacter)
	{
		CharacterMovementComponent = OwnerPlayerCharacter->GetCharacterMovement();
	}
}

void UINSCharacterAimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (OwnerPlayerCharacter && CharacterMovementComponent && !OwnerPlayerCharacter->GetIsCharacterDead() && OwnerPlayerCharacter->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		UpdatePredictFallingToLandAlpha();
		UpdateDirection();
		UpdateHorizontalSpeed();
		UpdateVerticalSpeed();
		UpdatePitchAndYaw();
		UpdateWeaponIkSwayRotation(DeltaSeconds);
		UpdateIsMoving();
		UpdateSprintPlaySpeed();
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
	if (OwnerPlayerCharacter->GetIsCharacterDead())
	{
		UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("Owner character is dead,can't play animations"));
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
	return CurrentWeaponAnimData&&Montage_IsPlaying(CurrentWeaponAnimData->FPIdleAnim);
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
	UAnimMontage* CurrentMoveMontage = bIsAiming ?
		CurrentWeaponAnimData->FPMoveAnim :
		CurrentWeaponAnimData->FPAimMoveAnim;

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
	//FPS No need to update
	if (CurrentViewMode == EViewMode::TPS)
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



void UINSCharacterAimInstance::UpdatePredictFallingToLandAlpha()
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

void UINSCharacterAimInstance::UpdateWeaponIkSwayRotation(float deltaSeconds)
{
	if (CurrentViewMode == EViewMode::FPS && OwnerPlayerController)
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

void UINSCharacterAimInstance::PlayFireAnim(bool bHasForeGrip, bool bIsDry)
{
	if (!CheckValid())
	{
		return;
	}
	/*UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("received weapon %s fire event"), *CurrentWeaponRef->GetName());
	
	Montage_Play(CurrentWeaponAnimData->FPFireHandsRecoilAnims);
	Montage_Play(CurrentWeaponAnimData->FireAnimFPTP.FireSwayAim);
	if (!GetIsAiming())
	{
		const uint8 FireRecoilMontageNum = CurrentWeaponAsstetsRef->FireAnimFPTP.CharFireRecoilMontages.Num();
		if (FireRecoilMontageNum > 0)
		{
			const uint8 RandomIndex = FMath::RandHelper(FireRecoilMontageNum - 1);
			Montage_Play(CurrentWeaponAsstetsRef->FireAnimFPTP.CharFireRecoilMontages[RandomIndex]);
		}
	}
	else
	{
		const uint8 FireRecoilMontageADSNum = CurrentWeaponAsstetsRef->FireAnimFPTP.CharFireRecoilMontagesADS.Num();
		if (FireRecoilMontageADSNum > 0)
		{
			const uint8 RandomIndex = FMath::RandHelper(FireRecoilMontageADSNum - 1);
			Montage_Play(CurrentWeaponAsstetsRef->FireAnimFPTP.CharFireRecoilMontagesADS[RandomIndex]);
		}
	}*/
}

void UINSCharacterAimInstance::PlayReloadAnim(bool bHasForeGrip, bool bIsDry)
{
	if (!CheckValid())
	{
		return;
	}
	/*UE_LOG(LogINSCharacterAimInstance,
		Warning,
		TEXT("character:%s received weapon:%s Start reload event"),
		*OwnerPlayerCharacter->GetName(),
		*CurrentWeaponRef->GetName());
	bHasForeGrip = CurrentWeapon->bForeGripEquipt;
	bIsDry = CurrentWeapon->bDryReload;

	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* SelectedFPReloadMontage = nullptr;
		if (bHasForeGrip && !bIsDry)
		{
			SelectedFPReloadMontage = CurrentWeaponAnimData->ReloadForeGripFP.CharReloadModeMontage;
		}
		if (bHasForeGrip && bIsDry)
		{
			SelectedFPReloadMontage = CurrentWeaponAnimData->ReloadDryForeGripFP.CharReloadDryModeMontage;
		}
		if (!bHasForeGrip && !bIsDry)
		{
			SelectedFPReloadMontage = CurrentWeaponAnimData->ReloadAltGripFP.CharReloadModeMontage;
		}
		if (!bHasForeGrip && bIsDry)
		{
			SelectedFPReloadMontage = CurrentWeaponAnimData->ReloadDryAltGripFP.CharReloadDryModeMontage;
		}
		if (!SelectedFPReloadMontage)
		{
			UE_LOG(LogINSCharacterAimInstance,
				Warning,
				TEXT("character %s In FPS view mode Is trying to play Reload montage,but selectd reload Montage is missing,abort!!!"),
				*OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedFPReloadMontage);
		UE_LOG(LogINSCharacterAimInstance,
			Log,
			TEXT("character %s In FPS view mode Is  playing Reload montage,reload Montage Name is %s"),
			*OwnerPlayerCharacter->GetName(),
			*SelectedFPReloadMontage->GetName());
	}
	else if (CurrentViewMode == EViewMode::TPS)
	{
		UAnimMontage* SelectedTPReloadMontage = nullptr;
		if (bHasForeGrip && !bIsDry)
		{
			SelectedTPReloadMontage = CurrentWeaponAnimData->ReloadForeGripTP.CharReloadModeMontage;
		}
		else if (bHasForeGrip && bIsDry)
		{
			SelectedTPReloadMontage = CurrentWeaponAnimData->ReloadDryForeGripTP.CharReloadDryModeMontage;
		}
		else if (!bHasForeGrip && !bIsDry)
		{
			SelectedTPReloadMontage = CurrentWeaponAnimData->ReloadAltGripTP.CharReloadModeMontage;
		}
		else if (!bHasForeGrip && bIsDry)
		{
			SelectedTPReloadMontage = CurrentWeaponAnimData->ReloadDryAltGripTP.CharReloadDryModeMontage;
		}
		if (!SelectedTPReloadMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning
				, TEXT("character %s In TPS view mode Is trying to play Reload montage,but selectd reload Montage is missing,abort!!!"),
				*OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedTPReloadMontage);
		UE_LOG(LogINSCharacterAimInstance,
			Log,
			TEXT("character %s In TPS view mode Is playing Reload montage,reload Montage Name is %s"),
			*OwnerPlayerCharacter->GetName(),
			*SelectedTPReloadMontage->GetName());
	}*/
}

void UINSCharacterAimInstance::PlaySwitchFireModeAnim(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	/*UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character:%s received weapon:%s switch fire mode event"),
		*OwnerPlayerCharacter->GetName(),
		*CurrentWeaponRef->GetName());
	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* SelectedFPFireModeSwitchMontage = nullptr;
		if (bHasForeGrip)
		{
			SelectedFPFireModeSwitchMontage = CurrentWeaponAnimData->FireModeSwitchForeGripFP.CharSwitchFireModeMontage;
		}
		else
		{
			SelectedFPFireModeSwitchMontage = CurrentWeaponAnimData->FireModeSwitchAltGripFP.CharSwitchFireModeMontage;
		}
		if (!SelectedFPFireModeSwitchMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play FireMode Switch montage,but selectd FireMode Switch montage is missing,abort!!!"),
				*OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedFPFireModeSwitchMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing FireMode Switch montage,FireMode Switch montage Name is %s"),
			*OwnerPlayerCharacter->GetName(),
			*SelectedFPFireModeSwitchMontage->GetName());
	}
	else if (CurrentViewMode == EViewMode::TPS)
	{
		UAnimMontage* SelectedTPFireModeSwitchMontage = nullptr;
		if (bHasForeGrip)
		{
			SelectedTPFireModeSwitchMontage = CurrentWeaponAnimData->FireModeSwitchForeGripTP.CharSwitchFireModeMontage;
		}
		else
		{
			SelectedTPFireModeSwitchMontage = CurrentWeaponAnimData->FireModeSwitchAltGripTP.CharSwitchFireModeMontage;
		}
		if (!SelectedTPFireModeSwitchMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play FireMode Switch montage,but selectd FireMode Switch montage is missing,abort!!!"),
				*OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedTPFireModeSwitchMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing FireMode Switch montage,FireMode Switch montage Name is %s"),
			*OwnerPlayerCharacter->GetName(),
			*SelectedTPFireModeSwitchMontage->GetName());
	}*/
}

void UINSCharacterAimInstance::PlayWeaponBasePose(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	//if (CurrentViewMode == EViewMode::FPS)
	//{
	//	UAnimMontage* SelectedBaseFPWeaponAnimMontage = nullptr;
	//	bHasForeGrip = CurrentWeapon->bForeGripEquipt;

	//	if (bHasForeGrip)
	//	{
	//		SelectedBaseFPWeaponAnimMontage = CurrentWeaponAnimData->BasePoseForeGripFP.CharBasePoseMontage;
	//	}
	//	else if (!bHasForeGrip)
	//	{
	//		SelectedBaseFPWeaponAnimMontage = CurrentWeaponAnimData->BasePoseAltGripFP.CharBasePoseMontage;
	//	}
	//	if (!SelectedBaseFPWeaponAnimMontage)
	//	{
	//		UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play weapon base pose montage,but selectd base pose  montage is missing,abort!!!"),
	//			*OwnerPlayerCharacter->GetName());
	//		return;
	//	}
	//	Montage_Play(SelectedBaseFPWeaponAnimMontage);
	//	//UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing FireMode Switch montage,FireMode Switch montage Name is %s"), *OwnerPlayerCharacter->GetName(), *SelectedBaseFPWeaponAnimMontage->GetName());
	//}

	////@TODO Character stance,Stand,crouch,prone
	//if (CurrentViewMode == EViewMode::TPS)
	//{
	//	UAnimMontage* SelectedBaseTPWeaponAnimMontage = nullptr;
	//	bHasForeGrip = CurrentWeapon->bForeGripEquipt;
	//	if (bHasForeGrip)
	//	{
	//		SelectedBaseTPWeaponAnimMontage = CurrentWeaponAnimData->BasePoseForeGripTP.CharBasePoseMontage;
	//	}
	//	else if (!bHasForeGrip)
	//	{
	//		SelectedBaseTPWeaponAnimMontage = CurrentWeaponAnimData->BasePoseAltGripTP.CharBasePoseMontage;
	//	}
	//	if (!SelectedBaseTPWeaponAnimMontage)
	//	{
	//		UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In TPS view mode Is trying to play weapon base pose montage,but selectd base pose  montage is missing,abort!!!"),
	//			*OwnerPlayerCharacter->GetName());
	//		return;
	//	}
	//	Montage_Play(SelectedBaseTPWeaponAnimMontage);
	//}
}

void UINSCharacterAimInstance::PlayWeaponStartEquipAnim(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	/*UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character:%s received weapon:%s start equip event"),
		*OwnerPlayerCharacter->GetName(),
		*CurrentWeaponRef->GetName());
	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* SelectedFPEquipMontage = nullptr;
		if (bHasForeGrip)
		{
			SelectedFPEquipMontage = CurrentWeaponAnimData->EquipForeGripFP.CharEquipMontage;
		}
		else
		{
			SelectedFPEquipMontage = CurrentWeaponAnimData->EquipAltGripFP.CharEquipMontage;
		}
		if (!SelectedFPEquipMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning,
				TEXT("character %s In FPS view mode Is trying to play weapon equip montage,but selectd deploy montage is missing,abort!!!"),
				*OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedFPEquipMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing weapon equip montage, montage Name is %s"),
			*OwnerPlayerCharacter->GetName(),
			*SelectedFPEquipMontage->GetName());
	}
	else if (CurrentViewMode == EViewMode::TPS)
	{
		UAnimMontage* SelectedTPEquipMontage = nullptr;
		if (bHasForeGrip)
		{
			SelectedTPEquipMontage = CurrentWeaponAnimData->EquipForeGripTP.CharEquipMontage;
		}
		else if (!bHasForeGrip)
		{
			SelectedTPEquipMontage = CurrentWeaponAnimData->EquipAltGripTP.CharEquipMontage;
		}
		if (!SelectedTPEquipMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play weapon equip montage,but selectd deploy montage is missing,abort!!!"),
				*OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedTPEquipMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing weapon equip montage, montage Name is %s"),
			*OwnerPlayerCharacter->GetName(),
			*SelectedTPEquipMontage->GetName());
	}*/
}

void UINSCharacterAimInstance::PlayAimAnim()
{
	bIsAiming = true;
}

void UINSCharacterAimInstance::PlayStopAimAnim()
{
	bIsAiming = false;
}

void UINSCharacterAimInstance::PlaySprintAnim()
{
	bIsSprinting = true;
	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* const SelectedSprintMontage = CurrentWeaponAnimData->FPSprintAnim;
		if (!Montage_IsPlaying(SelectedSprintMontage))
		{
			Montage_Play(SelectedSprintMontage);
		}
	}
}

void UINSCharacterAimInstance::StopPlaySprintAnim()
{
	bIsSprinting = false;
	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* const SelectedSprintMontage = CurrentWeaponAnimData->FPSprintAnim;
		if (Montage_IsPlaying(SelectedSprintMontage))
		{
			Montage_Stop(0.3f, SelectedSprintMontage);
		}
	}
}

void UINSCharacterAimInstance::OnWeaponAnimDelegateBindingFinished()
{
	bWeaponAnimDelegateBindingFinished = true;
	PlayWeaponStartEquipAnim(CurrentWeapon->bForeGripEquipt);
}

void UINSCharacterAimInstance::FPPlayWeaponIdleAnim()
{
	if (!CheckValid())
	{
		return;
	}

	if (!IsFPPlayingWeaponIdleAnim())
	{
		Montage_Play(CurrentWeaponAnimData->FPIdleAnim);
	}
}

void UINSCharacterAimInstance::PlayWeaponIdleAnim()
{
	if (CurrentViewMode == EViewMode::FPS)
	{
		FPPlayWeaponIdleAnim();
	}
}

void UINSCharacterAimInstance::SetIsAiming(bool IsAiming)
{
	bIsAiming = IsAiming;
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



void UINSCharacterAimInstance::SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon)
{
	CurrentWeapon = NewWeapon;
	CurrentWeaponAnimData = CurrentWeapon == nullptr
		? nullptr
		: CurrentWeapon->GetWeaponAnim();
}

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
void UINSCharacterAimInstance::AddScreenAminDebugMessage(const UAnimMontage* const Anim)
{

}
#endif