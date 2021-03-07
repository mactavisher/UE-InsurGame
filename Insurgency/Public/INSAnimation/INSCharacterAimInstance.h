// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Insurgency/Insurgency.h"
#include "INSAssets/INSWeaponAssets.h"
#include "INSInterfaces/INSWeaponAnimInterface.h"
#include "INSCharacterAimInstance.generated.h"

class UBlendSpace;
class AINSCharacter;
class AINSPlayerCharacter;
class AINSPlayerController;
class UCharacterMovementComponent;
class UINSStaticAnimData;
class AINSWeaponBase;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSCharacterAimInstance, Log, All);

/**
 *
 */
UCLASS()
class INSURGENCY_API UINSCharacterAimInstance : public UAnimInstance, public IINSWeaponAnimInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
		uint8 bIsMoving : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
		uint8 bIsFalling : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
		uint8 bIsInAir : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
		uint8 bIsSprinting : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
		uint8 bIsArmed : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFCharacterAnim|State")
		uint8 bIsAiming : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		uint8 bIsCrouching : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		uint8 bStartJump : 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		uint8 bSprintPressed : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float Yaw;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float Pitch;

	/** is this character is landed ,default is landed ,which is 1.0f */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float CustomNotIsFallingAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		float LeftHandIkAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		float RightHandIkAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blend")
		float HorizontalSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blend")
		float VerticalSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blend")
		float SprintPlaySpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FVector WeaponIKRootOffSetEffector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FVector BaseHandIKEffector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FVector ADSHandIKEffector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FVector CurrentHandIKEffector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FVector WeaponIKLeftHandOffSetEffector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FVector WeaponIKRightHandOffSetEffector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FRotator WeaponIKSwayRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		float  WeaponIKSwayRotationAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
		EViewMode CurrentViewMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float Direction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
		EWeaponBasePoseType CurrentWeaponBaseType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stance")
		ECharacterStance CurrentStance;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimDelegate")
		uint8 bWeaponAnimDelegateBindingFinished : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "INSCharacter|Debug")
		uint8 bShowDebugTrace : 1;
#endif

protected:
	/** cached player character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UCharacterMovementComponent* CharacterMovementComponent;

	/** cached weapon that currently in use */
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
		AINSWeaponBase* CurrentWeapon;

	/** cached player Animation data*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UINSStaticAnimData* CurrentWeaponAnimData;

	/** cached player character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		AINSCharacter* OwnerPlayerCharacter;


public:
	virtual void SetWeaponBasePoseType(EWeaponBasePoseType NewBasePoseType) { this->CurrentWeaponBaseType = NewBasePoseType; }

	virtual bool GetIsFalling()const { return bIsFalling; };

	virtual void SetIsSprinting(bool bIsSprintingNow) { this->bIsSprinting = bIsSprintingNow; }

	virtual void SetSprintPressed(bool NewSprintPressed) { this->bSprintPressed = NewSprintPressed; }

	virtual void SetIsCrouching(bool bIsCrouchingNow) { this->bIsCrouching = bIsCrouchingNow; }

	virtual void SetIsArmed(bool NewAremdState) { this->bIsArmed = NewAremdState; }

	virtual void SetIsAiming(bool IsAiming);

	virtual bool GetIsAiming()const { return bIsAiming; }

	virtual void SetStartJump(bool NewJumpState) { this->bStartJump = NewJumpState; }

	/**
	 * Set the currently used weapon and weapon animation data
	 * @Param NewWeapon   AINSWeaponBase
	 */
	virtual void SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon);

	/**
	 * Set the currently view mode could either be FPS or TPS
	 * @Param NewViewMode   EViewMode
	 */
	virtual void SetViewMode(EViewMode NewViewMode) { CurrentViewMode = NewViewMode; }


	/**
	 * Set the currently Stance pose to play
	 * @Param NewStance   ECharacterStance
	 */
	virtual void SetCurrentStance(ECharacterStance NewStance) { CurrentStance = NewStance; }


protected:
	/** native update for variables tick */
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

	/** native initialize Animation implementation */
	virtual void NativeInitializeAnimation()override;

	/** calculate horizontal speed */
	virtual void UpdateHorizontalSpeed();

	/** calculate horizontal speed */
	virtual void UpdateVerticalSpeed();

	/** perform a check before playing any kind of animation */
	virtual bool CheckValid();

	virtual bool IsFPPlayingWeaponIdleAnim();

	virtual void UpdateDirection();

	virtual void UpdatePitchAndYaw();

	virtual void UpdateIsAiming();

	virtual void UpdateWeaponBasePoseType();

	virtual void UpdateHandsIk();

	virtual void UpdateSprintPlaySpeed();

	virtual void UpdateIsMoving();

	virtual void UpdateIsFalling();

	//~ begin INSWeaponAnim Interface
public:

	virtual void PlayAimAnim()override;

	virtual void PlayStopAimAnim()override;

	virtual void PlaySprintAnim()override;

	virtual void StopPlaySprintAnim()override;

	UFUNCTION()
		virtual void StopFPPlayingWeaponIdleAnim();

	UFUNCTION()
		virtual void FPPlayMoveAnimation();

	UFUNCTION()
		virtual void StopFPPlayMoveAnimation();

	virtual void OnWeaponAnimDelegateBindingFinished()override;

	UFUNCTION()
		virtual void FPPlayWeaponIdleAnim();

	virtual void PlayWeaponIdleAnim()override;
	//~ end INSWeaponAnim Interface

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	virtual void AddScreenAminDebugMessage(const UAnimMontage* const Anim);
#endif

};
