// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSAnimation/INSCharacterAimInstance.h"
#include "INSFPAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class INSURGENCY_API UINSFPAnimInstance : public UINSCharacterAimInstance
{
	GENERATED_UCLASS_BODY()

protected:

	/** current ads alpha when aim, used to blend aim animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
		float ADSAlpha;

	/** current ads alpha when aim, used to blend aim animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
		float ADSTime;

	/** hands shift value when firing to simulate recoil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
		FVector FiringHandsShift;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="ADS")
	   UAnimSequence* AdjustableAdsPoseRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float MaxWeaponSwayPitch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float MaxWeaponSwayYaw;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float WeaponSwayRecoverySpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float WeaponSwayScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		FVector SightLocWhenAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
	    FTransform SightTransform;

	UPROPERTY()
	   uint8 bSighLocReCalculated:1;

	UPROPERTY()
	AINSPlayerController* OwnerPlayerController;

	/** timer handle for play aim idle animations entire enter aiming status */
	FTimerHandle AimIdleAnimTimer;

	/**
	 * update Animation Variables
	 * @Param DeltaSeconds  DeltaSeconds
	 */
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

	/**
	 * update ads alpha when aim
	 * @Param DeltaSeconds  DeltaSeconds
	 */
	virtual void UpdateAdsAlpha(float DeltaSeconds);

	/**
	 * @Desc Native Initialize Animation data
	 */
	virtual void NativeInitializeAnimation()override;

	virtual void UpdateWeaponIkSwayRotation(float deltaSeconds);

	virtual void FPCheckAndPlayIdleAnim();

	virtual void FPCheckAndPlayMovingAnim();

	virtual void UpdateSight();

	virtual void PlayWeaponStartEquipAnim()override;

	/**
	 * @Desc calculate sight transform when ads to match center of the screen
	 * 
	 * @Param OutRelativeTransform  produce the relative transform between sight and camera
	 */
	virtual void CalculateSight(FTransform& OutRelativeTransform);

	virtual void SetupAimIdleAnim(const bool NewAimingCondition);

	/**
	 * play the weapon base pose each frame to correct the weapon pose
	 */
	virtual void PlayWeaponBasePose()override;

	/**
	 * @Desc play weapon reload Animation
	 * @Param bIsDry is this a dry reload
	 */
	virtual void PlayReloadAnim(bool bIsDry)override;

	UFUNCTION()
	virtual void PlayAimIdleAnim();

	/**
	 * @Desc set is aiming and relative aiming variables
	 * @Param isAiming  aiming conditions
	 */
	virtual void SetIsAiming(bool IsAiming)override;

	virtual void SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon)override;

public:
	virtual void PlayFireAnim()override;

	/**
	 * update HandsShift when firing
	 * @Param DeltaSeconds  DeltaSeconds
	 */
	virtual void UpdateFiringHandsShift(float DeltaSeconds);
};
