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
		float MaxWeaponSwayDelta;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float WeaponSwaySpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float MaxWeaponSwayDeltaAimingModifier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float WeaponSwayLocationFactor;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		FVector SightLocWhenAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
	    FTransform SightTransform;

	UPROPERTY()
	   uint8 bSighLocReCalculated:1;

	UPROPERTY()
	FRotator LastRotation;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
    FVector WeaponSwayLocation;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	AINSPlayerController* OwnerPlayerController;

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

	virtual void NativeBeginPlay()override;

	virtual void FPStopIdleAnim();

	virtual void FPStopMoveAnim();

	virtual void FPStopAimMoveAnim();

	virtual void UpdateWeaponIkSwayRotation(float deltaSeconds);

	virtual void FPPlayMovingAnim();

	virtual void FPPlayIdleAnim();

	/**
	 * @desc when entering ads, calculate and adjust the hand IK to line up sight with camera
	 */
	virtual void UpdateSight();

	virtual float PlayWeaponStartEquipAnim()override;

	/**
	 * @Desc calculate sight transform when ads to match center of the screen
	 * 
	 * @Param OutRelativeTransform  produce the relative transform between sight and camera
	 */
	virtual void CalculateSight(FTransform& OutRelativeTransform);

	/**
	 * play the weapon base pose each frame to correct the weapon pose
	 */
	virtual float PlayWeaponBasePose()override;

	/**
	 * @Desc play weapon reload Animation
	 * @Param bIsDry is this a dry reload
	 * @return the animation duration 
	 */
	virtual float PlayReloadAnim(bool bIsDry)override;

	/**
	 * @Desc set is aiming and relative aiming variables
	 * @Param isAiming  aiming conditions
	 */
	virtual void SetIsAiming(bool IsAiming)override;

	virtual void SetIsSprinting(bool bIsSprintingNow)override;

	virtual void SetSprintPressed(bool NewSprintPressed)override;

	virtual void SetIdleState(bool NewIdleState)override;

	virtual void SetBoredState(bool NewBoredState)override;

	virtual void PlayBoredAnim()override;

	virtual float PlaySwitchFireModeAnim() override;

	virtual void SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon)override;

public:
	virtual float PlayFireAnim()override;

	/**
	 * update HandsShift when firing
	 * @Param DeltaSeconds  DeltaSeconds
	 */
	virtual void UpdateFiringHandsShift(float DeltaSeconds);
};
