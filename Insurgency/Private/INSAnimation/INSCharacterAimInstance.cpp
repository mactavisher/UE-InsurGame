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

DEFINE_LOG_CATEGORY(LogINSCharacterAimInstance);

UINSCharacterAimInstance::UINSCharacterAimInstance(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bWeaponAnimDelegateBindingFinished = false;
	bIsAiming = false;
	StandStopMoveAlpha = 0.f;
	CanEnterSprint = false;
	CanStopSprint = true;
	Pitch = 0.f;
	Yaw = 0.5f;
	Direction = 0.f;
	CustomNotIsFallingAlpha = 1.f;
	LeftHandIkAlpha = 1.f;
	RightHandIkAlpha = 1.f;
	JogPlayRate = 1.0f;
	WalkPlayRate = 1.0f;
	WeaponIKRootOffSetEffector = FVector::ZeroVector;
	WeaponIKLeftHandOffSetEffector = FVector::ZeroVector;
	WeaponIKRightHandOffSetEffector = FVector::ZeroVector;
	WeaponIKSwayRotation = FRotator::ZeroRotator;
	bIsMoving = false;
	bStartJump = false;
	bIsCrouching = false;
	WalkToStopAlpha = 1.f;
	SprintToWalkAlpha = 1.0f;
	ADSAlpha = 0.f;
	WeaponIKSwayRotationAlpha = 0.f;
	TPShouldTurnLeft90 = false;
	TPShouldTurnRight90 = bIsFalling;
	bIsAiming = false;
	bIsTurning = false;
	CurrentViewMode = EViewMode::FPS;
	MaxWeaponSwayPitch = 5.f;
	MaxWeaponSwayYaw = MaxWeaponSwayPitch;
	WeaponSwayRecoverySpeed = 40.f;
	WeaponSwayScale = 10.f;
	TPShouldTurnLeft90 = false;
	CurrentStance = ECharacterStance::STAND;
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

void UINSCharacterAimInstance::UpdateSprintToWalkAlpha()
{
	if (OwnerPlayerCharacter->GetIsSprint())
	{
		const float CurrentSpeed = CharacterMovementComponent->GetLastUpdateVelocity().Size();
		const float MaxWalkSpeed = CharacterMovementComponent->GetMaxSpeed();
		SprintToWalkAlpha = CurrentSpeed / MaxWalkSpeed;
	}
	else
	{
		SprintToWalkAlpha = 1.f;
	}
}

void UINSCharacterAimInstance::UpdateTurnConditions()
{
	if (GetOwningComponent() && CurrentViewMode == EViewMode::TPS)
	{
		const float DeltaYaw = OwnerPlayerCharacter->GetControlRotation().Yaw - GetOwningComponent()->GetForwardVector().Rotation().Yaw;
		//UE_LOG(LogINSCharacterAimInstance, Log, TEXT("DeltaYaw value with mesh and controll:%f"), DeltaYaw);
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

void UINSCharacterAimInstance::UpdateADSAlpha(float DeltaTimeSeconds)
{
	if (CurrentWeaponRef == nullptr)
	{
		return;
	}
	const float WeaponAimTime = CurrentWeaponRef->GetWeaponAimTime();
	const float InterpSpeed = 1.f / WeaponAimTime;
	if (bIsAiming)
	{
		ADSAlpha = ADSAlpha + InterpSpeed * DeltaTimeSeconds;
		if (ADSAlpha >= 1.f)
		{
			ADSAlpha = 1.0f;
		}
		if (ADSAlpha==1.f&&CurrentWeaponRef)
		{
			if (ADSHandIKEffector.IsZero())
			{
				FTransform CameraTrans = Cast<AINSPlayerCharacter>(OwnerPlayerCharacter)->GetPlayerCameraTransform();
				FTransform WeaponSightTrans = CurrentWeaponRef->GetSightsTransform();
				FVector RelLoc = UKismetMathLibrary::MakeRelativeTransform(CameraTrans, WeaponSightTrans).GetLocation();
				//FVector RelLoc = CameraTrans.GetLocation() - WeaponSightTrans.GetLocation();
				ADSHandIKEffector = FVector(-10.f, RelLoc.Y, RelLoc.Z);
			}
			CurrentHandIKEffector.X = FMath::Clamp<float>(CurrentHandIKEffector.X + DeltaTimeSeconds*0.1f, CurrentHandIKEffector.X, ADSHandIKEffector.X);
			CurrentHandIKEffector.Y = FMath::Clamp<float>(CurrentHandIKEffector.Y + DeltaTimeSeconds*0.1f, CurrentHandIKEffector.Y, ADSHandIKEffector.Y);
			CurrentHandIKEffector.Z = FMath::Clamp<float>(CurrentHandIKEffector.Z + DeltaTimeSeconds*0.1f, CurrentHandIKEffector.Z, ADSHandIKEffector.Z);
		}
	}
	else
	{
		ADSAlpha = ADSAlpha - InterpSpeed * DeltaTimeSeconds;
		if (ADSAlpha <= 0.f)
		{
			ADSAlpha = 0.f;
		}
		ADSHandIKEffector = FVector(ForceInit);
		CurrentHandIKEffector.X = FMath::Clamp<float>(CurrentHandIKEffector.X - DeltaTimeSeconds*0.1f, 0.f, CurrentHandIKEffector.X);
		CurrentHandIKEffector.Y = FMath::Clamp<float>(CurrentHandIKEffector.Y - DeltaTimeSeconds*0.1f, 0.f, CurrentHandIKEffector.Y);
		CurrentHandIKEffector.Z = FMath::Clamp<float>(CurrentHandIKEffector.Z - DeltaTimeSeconds*0.1f, 0.f, CurrentHandIKEffector.Z);
	}
}

void UINSCharacterAimInstance::SetCurrentWeaponRef(class AINSWeaponBase* NewWeaponRef)
{
	CurrentWeaponRef = NewWeaponRef;
	CurrentWeaponAsstetsRef = NewWeaponRef == nullptr ? nullptr : NewWeaponRef->GetWeaponAssets();
	if (CurrentWeaponRef)
	{
		AimAnimdata = CurrentWeaponAsstetsRef->AimAnimFP;
		StandWalkBlendSpace = CurrentWeaponAsstetsRef->StandWalkType;
		CrouchWalkBlendSpace = CurrentWeaponAsstetsRef->CrouchWalkType;
		StandJogBlendSpace = CurrentWeaponAsstetsRef->StandJogType;
		CrouchJogBlendSpace = CurrentWeaponAsstetsRef->CrouchJogType;
		BindWeaponAnimDelegate();
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
		UpdateADSAlpha(DeltaSeconds);
		UpdateWeaponIkSwayRotation(DeltaSeconds);
		UpdateStandStopMoveAlpha();
		UpdateTurnConditions();
		if (!OwnerPlayerController)
		{
			OwnerPlayerController = Cast<AINSPlayerController>(OwnerPlayerCharacter->GetController());
		}
		if (CurrentWeaponRef)
		{
			PlayWeaponBasePose(CurrentWeaponRef->GetIsWeaponHasForeGrip());
			if (bIsAiming&&CurrentWeaponAsstetsRef&&Montage_IsPlaying(CurrentWeaponAsstetsRef->IdleAnimFP.CharIdleMontage))
			{
				Montage_Stop(0.2f, CurrentWeaponAsstetsRef->IdleAnimFP.CharIdleMontage);
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
}

void UINSCharacterAimInstance::UpdateHorizontalSpeed()
{
	HorizontalSpeed = OwnerPlayerCharacter->GetVelocity().Size2D();
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
	if (!bWeaponAnimDelegateBindingFinished)
	{
		UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("waiting for delegate binding finished,can't play animations"));
		return false;
	}
	if (!CurrentWeaponRef)
	{
		UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("Missing Current Weapon Ref,invalid for playing any weapon anim"));
		return false;
	}
	if (!CurrentWeaponAsstetsRef)
	{
		UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("Missing Current Weapon asstes Ref,invalid for playing any weapon anim"));
		return false;
	}
	return true;
}

bool UINSCharacterAimInstance::IsFPPlayingWeaponIdleAnim()
{
	return Montage_IsPlaying(CurrentWeaponRef->GetWeaponAssets()->IdleAnimFP.CharIdleMontage);
}

void UINSCharacterAimInstance::StopFPPlayingWeaponIdleAnim()
{
	if (CurrentWeaponRef && CurrentWeaponRef->GetWeaponAssets())
	{
		if (IsFPPlayingWeaponIdleAnim())
		{
			Montage_Stop(0.2f, CurrentWeaponRef->GetWeaponAssets()->IdleAnimFP.CharIdleMontage);
		}
	}
}

void UINSCharacterAimInstance::FPPlayMoveAnimation()
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* CurrentMoveMontage = bIsAiming ? CurrentWeaponAsstetsRef->MoveAnim1P.AimMoveMontage : CurrentWeaponAsstetsRef->MoveAnim1P.MoveMontage;
	UAnimMontage* AimMoveMontage = CurrentWeaponAsstetsRef->MoveAnim1P.AimMoveMontage;
	UAnimMontage* MoveMontage = CurrentWeaponAsstetsRef->MoveAnim1P.MoveMontage;
	const bool bIsPlaying1pMoveAnim = Montage_IsPlaying(CurrentMoveMontage);
	//if no montage player currently, just play it
	if (!bIsPlaying1pMoveAnim)
	{
		Montage_Play(CurrentMoveMontage);
		//UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Character %s Is Playing Reload animation,animation name:%s"), *CurrentMoveMontage->GetName());
	}
	if (bIsAiming&&Montage_IsPlaying(MoveMontage))
	{
		Montage_Stop(0.2f, MoveMontage);
		Montage_Play(AimMoveMontage);
	}
	if (!bIsAiming&&Montage_IsPlaying(AimMoveMontage))
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
	UAnimMontage* MoveMontage = CurrentWeaponAsstetsRef->MoveAnim1P.MoveMontage;
	UAnimMontage* AimMoveMontage = CurrentWeaponAsstetsRef->MoveAnim1P.AimMoveMontage;
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

void UINSCharacterAimInstance::UpdateStandStopMoveAlpha()
{
	if (CharacterMovementComponent)
	{
		float CharacterCurrentSpeed = CharacterMovementComponent->GetLastUpdateVelocity().Size2D();
		float CharacterMaxMoveSpeed = CharacterMovementComponent->MaxWalkSpeed;
		WalkToStopAlpha = CharacterCurrentSpeed / CharacterMaxMoveSpeed;
	}
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
			Yaw = UKismetMathLibrary::MapRangeClamped(ControlRotation.Yaw, 0.f, 90.f, 0.5f, 0.f)*1.067f;
		}
		if (ControlRotation.Yaw < 0.f)
		{
			Yaw = UKismetMathLibrary::MapRangeClamped(ControlRotation.Yaw, 0.f, -90.f, 0.5f, 1.f)*1.067f;
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

void UINSCharacterAimInstance::UpdatePredictFallingToLandAlpha()
{
	// from this distance we start to update falling Alpha value ,any distance beyond this value will ignore and falling alpha is 0
	// means this character is full falling state and not prepared to land,with trace start hit frame , prepare to land and from where start 
	// to update falling alpha ,used for land and moving animation blending
	const float FallingCalMinDistance = 30.f;
	if (CharacterMovementComponent->IsFalling())
	{
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwnerPlayerCharacter);
		const FVector CharacterCurrentLocation = OwnerPlayerCharacter->GetActorLocation();
		const FVector TraceStartLocation = FVector(CharacterCurrentLocation.X, CharacterCurrentLocation.Y, CharacterCurrentLocation.Z - FMath::Abs(OwnerPlayerCharacter->BaseEyeHeight));
		const FVector TraceEndLocation = TraceStartLocation + FVector::DownVector * FallingCalMinDistance;
		FHitResult LandPredictHit(ForceInit);
		GetWorld()->LineTraceSingleByChannel(LandPredictHit, TraceStartLocation, TraceEndLocation, ECollisionChannel::ECC_Visibility, QueryParams);
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
	else
	{
		CustomNotIsFallingAlpha = 1.f;
	}
}

void UINSCharacterAimInstance::UpdateWeaponIkSwayRotation(float deltaSeconds)
{
	if (CurrentViewMode == EViewMode::FPS&&OwnerPlayerController)
	{
		const float RotYaw = OwnerPlayerController->GetInputAxisValue(TEXT("Turn"));
		const float RotPitch = OwnerPlayerController->GetInputAxisValue(TEXT("LookUp"));
		if (RotYaw == 0.f)
		{
			if (WeaponIKSwayRotation.Yaw > 0.f)
			{
				WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(WeaponIKSwayRotation.Yaw -= GetWorld()->GetDeltaSeconds()*WeaponSwayRecoverySpeed, 0.f, MaxWeaponSwayYaw);
			}
			if (WeaponIKSwayRotation.Yaw < 0.f)
			{
				WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(WeaponIKSwayRotation.Yaw += GetWorld()->GetDeltaSeconds()*WeaponSwayRecoverySpeed, -MaxWeaponSwayYaw, 0.f);
			}
		}
		else
		{
			WeaponIKSwayRotation.Yaw = FMath::Clamp<float>(WeaponIKSwayRotation.Yaw += GetWorld()->GetDeltaSeconds()*RotYaw*WeaponSwayScale, -MaxWeaponSwayYaw, MaxWeaponSwayYaw);
		}
		if (RotPitch == 0.f)
		{
			if (WeaponIKSwayRotation.Pitch > 0.f)
			{
				WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(WeaponIKSwayRotation.Pitch -= GetWorld()->GetDeltaSeconds()*WeaponSwayRecoverySpeed, 0.f, MaxWeaponSwayPitch);
			}
			if (WeaponIKSwayRotation.Pitch < 0.f)
			{
				WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(WeaponIKSwayRotation.Pitch += GetWorld()->GetDeltaSeconds()*WeaponSwayRecoverySpeed, -MaxWeaponSwayPitch, 0.f);
			}
		}
		else
		{
			WeaponIKSwayRotation.Pitch = FMath::Clamp<float>(WeaponIKSwayRotation.Pitch -= GetWorld()->GetDeltaSeconds()*RotPitch*WeaponSwayScale, -MaxWeaponSwayPitch, MaxWeaponSwayPitch);
		}
	}
}

void UINSCharacterAimInstance::PlayFireAnim(bool bHasForeGrip, bool bIsDry)
{
	if (!CheckValid())
	{
		return;
	}
	UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("received weapon %s fire event"), *CurrentWeaponRef->GetName());
	Montage_Play(CurrentWeaponAsstetsRef->FireAnimFPTP.CharPullTriggerMontage);
	Montage_Play(CurrentWeaponAsstetsRef->FireAnimFPTP.FireSwayAim);
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
	}
}

void UINSCharacterAimInstance::PlayReloadAnim(bool bHasForeGrip, bool bIsDry)
{
	if (!CheckValid())
	{
		return;
	}
	UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character:%s received weapon:%s Start reload event"), *OwnerPlayerCharacter->GetName(), *CurrentWeaponRef->GetName());
	bHasForeGrip = CurrentWeaponRef->bForeGripEquipt;
	bIsDry = CurrentWeaponRef->bDryReload;
	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* SelectedFPReloadMontage = nullptr;
		if (bHasForeGrip && !bIsDry)
		{
			SelectedFPReloadMontage = CurrentWeaponAsstetsRef->ReloadForeGripFP.CharReloadModeMontage;
		}
		if (bHasForeGrip && bIsDry)
		{
			SelectedFPReloadMontage = CurrentWeaponAsstetsRef->ReloadDryForeGripFP.CharReloadDryModeMontage;
		}
		if (!bHasForeGrip && !bIsDry)
		{
			SelectedFPReloadMontage = CurrentWeaponAsstetsRef->ReloadAltGripFP.CharReloadModeMontage;
		}
		if (!bHasForeGrip && bIsDry)
		{
			SelectedFPReloadMontage = CurrentWeaponAsstetsRef->ReloadDryAltGripFP.CharReloadDryModeMontage;
		}
		if (!SelectedFPReloadMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play Reload montage,but selectd reload Montage is missing,abort!!!"), *OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedFPReloadMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is  playing Reload montage,reload Montage Name is %s"), *OwnerPlayerCharacter->GetName(), *SelectedFPReloadMontage->GetName());
	}
	else if (CurrentViewMode == EViewMode::TPS)
	{
		UAnimMontage* SelectedTPReloadMontage = nullptr;
		if (bHasForeGrip && !bIsDry)
		{
			SelectedTPReloadMontage = CurrentWeaponAsstetsRef->ReloadForeGripTP.CharReloadModeMontage;
		}
		else if (bHasForeGrip && bIsDry)
		{
			SelectedTPReloadMontage = CurrentWeaponAsstetsRef->ReloadDryForeGripTP.CharReloadDryModeMontage;
		}
		else if (!bHasForeGrip && !bIsDry)
		{
			SelectedTPReloadMontage = CurrentWeaponAsstetsRef->ReloadAltGripTP.CharReloadModeMontage;
		}
		else if (!bHasForeGrip && bIsDry)
		{
			SelectedTPReloadMontage = CurrentWeaponAsstetsRef->ReloadDryAltGripTP.CharReloadDryModeMontage;
		}
		if (!SelectedTPReloadMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In TPS view mode Is trying to play Reload montage,but selectd reload Montage is missing,abort!!!"), *OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedTPReloadMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In TPS view mode Is playing Reload montage,reload Montage Name is %s"), *OwnerPlayerCharacter->GetName(), *SelectedTPReloadMontage->GetName());
	}
}

void UINSCharacterAimInstance::PlaySwitchFireModeAnim(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character:%s received weapon:%s switch fire mode event"), *OwnerPlayerCharacter->GetName(), *CurrentWeaponRef->GetName());
	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* SelectedFPFireModeSwitchMontage = nullptr;
		if (bHasForeGrip)
		{
			SelectedFPFireModeSwitchMontage = CurrentWeaponAsstetsRef->FireModeSwitchForeGripFP.CharSwitchFireModeMontage;
		}
		else
		{
			SelectedFPFireModeSwitchMontage = CurrentWeaponAsstetsRef->FireModeSwitchAltGripFP.CharSwitchFireModeMontage;
		}
		if (!SelectedFPFireModeSwitchMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play FireMode Switch montage,but selectd FireMode Switch montage is missing,abort!!!"), *OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedFPFireModeSwitchMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing FireMode Switch montage,FireMode Switch montage Name is %s"), *OwnerPlayerCharacter->GetName(), *SelectedFPFireModeSwitchMontage->GetName());
	}
	else if (CurrentViewMode == EViewMode::TPS)
	{
		UAnimMontage* SelectedTPFireModeSwitchMontage = nullptr;
		if (bHasForeGrip)
		{
			SelectedTPFireModeSwitchMontage = CurrentWeaponAsstetsRef->FireModeSwitchForeGripTP.CharSwitchFireModeMontage;
		}
		else
		{
			SelectedTPFireModeSwitchMontage = CurrentWeaponAsstetsRef->FireModeSwitchAltGripTP.CharSwitchFireModeMontage;
		}
		if (!SelectedTPFireModeSwitchMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play FireMode Switch montage,but selectd FireMode Switch montage is missing,abort!!!"), *OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedTPFireModeSwitchMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing FireMode Switch montage,FireMode Switch montage Name is %s"), *OwnerPlayerCharacter->GetName(), *SelectedTPFireModeSwitchMontage->GetName());
	}
}

void UINSCharacterAimInstance::PlayWeaponBasePose(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* SelectedBaseFPWeaponAnimMontage = nullptr;
		bHasForeGrip = CurrentWeaponRef->bForeGripEquipt;

		if (bHasForeGrip)
		{
			SelectedBaseFPWeaponAnimMontage = CurrentWeaponAsstetsRef->BasePoseForeGripFP.CharBasePoseMontage;
		}
		else if (!bHasForeGrip)
		{
			SelectedBaseFPWeaponAnimMontage = CurrentWeaponAsstetsRef->BasePoseAltGripFP.CharBasePoseMontage;
		}
		if (!SelectedBaseFPWeaponAnimMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play weapon base pose montage,but selectd base pose  montage is missing,abort!!!"), *OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedBaseFPWeaponAnimMontage);
		//UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing FireMode Switch montage,FireMode Switch montage Name is %s"), *OwnerPlayerCharacter->GetName(), *SelectedBaseFPWeaponAnimMontage->GetName());
	}

	//@TODO Character stance,Stand,crouch,prone
	if (CurrentViewMode == EViewMode::TPS)
	{
		UAnimMontage* SelectedBaseTPWeaponAnimMontage = nullptr;
		bHasForeGrip = CurrentWeaponRef->bForeGripEquipt;
		if (bHasForeGrip)
		{
			SelectedBaseTPWeaponAnimMontage = CurrentWeaponAsstetsRef->BasePoseForeGripTP.CharBasePoseMontage;
		}
		else if (!bHasForeGrip)
		{
			SelectedBaseTPWeaponAnimMontage = CurrentWeaponAsstetsRef->BasePoseAltGripTP.CharBasePoseMontage;
		}
		if (!SelectedBaseTPWeaponAnimMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In TPS view mode Is trying to play weapon base pose montage,but selectd base pose  montage is missing,abort!!!"), *OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedBaseTPWeaponAnimMontage);
	}
}

void UINSCharacterAimInstance::PlayWeaponStartEquipAnim(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character:%s received weapon:%s start equip event"), *OwnerPlayerCharacter->GetName(), *CurrentWeaponRef->GetName());
	if (CurrentViewMode == EViewMode::FPS)
	{
		UAnimMontage* SelectedFPEquipMontage = nullptr;
		if (bHasForeGrip)
		{
			SelectedFPEquipMontage = CurrentWeaponAsstetsRef->EquipForeGripFP.CharEquipMontage;
		}
		else
		{
			SelectedFPEquipMontage = CurrentWeaponAsstetsRef->EquipAltGripFP.CharEquipMontage;
		}
		if (!SelectedFPEquipMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play weapon equip montage,but selectd deploy montage is missing,abort!!!"), *OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedFPEquipMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing weapon equip montage, montage Name is %s"), *OwnerPlayerCharacter->GetName(), *SelectedFPEquipMontage->GetName());
	}
	else if (CurrentViewMode == EViewMode::TPS)
	{
		UAnimMontage* SelectedTPEquipMontage = nullptr;
		if (bHasForeGrip)
		{
			SelectedTPEquipMontage = CurrentWeaponAsstetsRef->EquipForeGripTP.CharEquipMontage;
		}
		else if (!bHasForeGrip)
		{
			SelectedTPEquipMontage = CurrentWeaponAsstetsRef->EquipAltGripTP.CharEquipMontage;
		}
		if (!SelectedTPEquipMontage)
		{
			UE_LOG(LogINSCharacterAimInstance, Warning, TEXT("character %s In FPS view mode Is trying to play weapon equip montage,but selectd deploy montage is missing,abort!!!"), *OwnerPlayerCharacter->GetName());
			return;
		}
		Montage_Play(SelectedTPEquipMontage);
		UE_LOG(LogINSCharacterAimInstance, Log, TEXT("character %s In FPS view mode Is playing weapon equip montage, montage Name is %s"), *OwnerPlayerCharacter->GetName(), *SelectedTPEquipMontage->GetName());
	}
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
		UAnimMontage* const SelectedSprintMontage = CurrentWeaponAsstetsRef->MoveAnim1P.SprintMontage;
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
		UAnimMontage* const SelectedSprintMontage = CurrentWeaponAsstetsRef->MoveAnim1P.SprintMontage;
		if (Montage_IsPlaying(SelectedSprintMontage))
		{
			Montage_Stop(0.3f, SelectedSprintMontage);
		}
	}
}

void UINSCharacterAimInstance::OnWeaponAnimDelegateBindingFinished()
{
	bWeaponAnimDelegateBindingFinished = true;
	PlayWeaponStartEquipAnim(CurrentWeaponRef->bForeGripEquipt);
}

void UINSCharacterAimInstance::FPPlayWeaponIdleAnim()
{
	if (!CheckValid())
	{
		return;
	}

	if (!IsFPPlayingWeaponIdleAnim())
	{
		Montage_Play(CurrentWeaponAsstetsRef->IdleAnimFP.CharIdleMontage);
	}
}

void UINSCharacterAimInstance::PlayWeaponIdleAnim()
{
	if (CurrentViewMode == EViewMode::FPS)
	{
		FPPlayWeaponIdleAnim();
	}
}

void UINSCharacterAimInstance::BindWeaponAnimDelegate()
{
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Start Binding Weapon Anim Delegate for Character"));
	FireDelegate.BindUFunction(this, TEXT("PlayFireAnim"));
	StartSwitchFireModeDelegate.BindUFunction(this, TEXT("PlaySwitchFireModeAnim"));
	StartEquipDelegate.BindUFunction(this, TEXT("PlayWeaponStartEquipAnim"));
	StartReloadDelegate.BindUFunction(this, TEXT("PlayReloadAnim"));
	StartWeaponIdleDelegate.BindUFunction(this, TEXT("PlayWeaponIdleAnim"));
	OutWeaponIdleDelegate.BindUFunction(this, TEXT("StopFPPlayingWeaponIdleAnim"));
	StartAimDelegate.BindUFunction(this, TEXT("PlayAimAnim"));
	StopAimDelegate.BindUFunction(this, TEXT("PlayStopAimAnim"));
	StartSprintDelegate.BindUFunction(this, TEXT("PlaySprintAnim"));
	StopSprintDelegate.BindUFunction(this, TEXT("StopPlaySprintAnim"));
	CurrentWeaponRef->OnWeaponAim.AddUnique(StartAimDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Bind Weapon Aim Anim delegate for character: Delegate Function::PlayAimAnim"));
	CurrentWeaponRef->OnStopWeaponAim.AddUnique(StopAimDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Bind Weapon Aim Anim delegate for character: Delegate Function::PlayAimAnim"));
	CurrentWeaponRef->OnWeaponStartEquip.AddUnique(StartEquipDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Bind Weapon Equip Anim delegate for character: Delegate Function::PlayWeaponStartEquipAnim"));
	CurrentWeaponRef->OnWeaponEachFire.AddUnique(FireDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Bind Weapon Fire Anim delegate for character: Delegate Function::PlayFireAnim"));
	CurrentWeaponRef->OnWeaponStartReload.AddUnique(StartReloadDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Bind Weapon Reload Anim delegate for character: Delegate Function::PlayReloadAnim"));
	CurrentWeaponRef->OnWeaponSwitchFireMode.AddUnique(StartSwitchFireModeDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Bind Weapon Switch Fire Mode Anim delegate for character: Delegate Function::PlaySwitchFireModeAnim"));
	CurrentWeaponRef->OnWeaponEnterIdle.AddUnique(StartWeaponIdleDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Bind Weapon Start Idle Anim delegate for character: Delegate Function::PlayWeaponIdleAnim"));
	CurrentWeaponRef->OnWeaponOutIdle.AddUnique(OutWeaponIdleDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Bind Weapon Out Idle Anim delegate for character: Delegate Function::StopCharacterPlayingWeaponIdleAnim"));
	OwnerPlayerCharacter->OnStartSprint.AddUnique(StartSprintDelegate);
	OwnerPlayerCharacter->OnStopSprint.AddUnique(StopSprintDelegate);
	OnWeaponAnimDelegateBindingFinished();
}

void UINSCharacterAimInstance::UnbindWeaponAnimDelegate()
{
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("Start unBinding Weapon Anim Delegate for Character"));
	CurrentWeaponRef->OnWeaponStartEquip.Remove(StartEquipDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("unBind Weapon Equip Anim delegate for character: Delegate Function::PlayWeaponStartEquipAnim"));
	CurrentWeaponRef->OnWeaponEachFire.Remove(FireDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("unBind Weapon Fire Anim delegate for character: Delegate Function::PlayFireAnim"));
	CurrentWeaponRef->OnWeaponStartReload.Remove(StartReloadDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("unBind Weapon Reload Anim delegate for character: Delegate Function::PlayReloadAnim"));
	CurrentWeaponRef->OnWeaponSwitchFireMode.Remove(StartSwitchFireModeDelegate);
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("unBind Weapon Switch Fire Mode Anim delegate for character: Delegate Function::PlaySwitchFireModeAnim"));
	UE_LOG(LogINSCharacterAimInstance, Log, TEXT("unBind unBinding Weapon Anim Delegate for Character"));
}

void UINSCharacterAimInstance::SetIsAiming(bool IsAiming)
{
	if (bIsAiming)
	{
		WeaponSwayScale = 2.f;
		MaxWeaponSwayPitch *= 0.2f;
		MaxWeaponSwayYaw *= 0.2f;
	}
	else
	{
		WeaponSwayScale = 5.f;
		MaxWeaponSwayPitch = 5.f;
		MaxWeaponSwayYaw = 5.f;
	}
}

void UINSCharacterAimInstance::UpdateWalkToStopAlpha()
{

}

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
void UINSCharacterAimInstance::AddScreenAminDebugMessage(const UAnimMontage* const Anim)
{

}
#endif