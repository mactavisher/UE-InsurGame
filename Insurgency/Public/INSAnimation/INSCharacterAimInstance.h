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
struct FAimAnim;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSCharacterAimInstance, Log, All);

/**
 *
 */
UCLASS()
class INSURGENCY_API UINSCharacterAimInstance : public UAnimInstance,public IINSWeaponAnimInterface
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
		uint8 bCanSprint : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
		uint8 bCanCrouch : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
		uint8 bIsArmed : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFCharacterAnim|State")
		uint8 bIsAiming : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		uint8 bIsCrouching : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		uint8 bStartJump : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float StandStopMoveAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		FVector LastInputVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		FRotator LastInputRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		FRotator DeltaRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		uint8 bIsTurning : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		uint8 bCanTurnInPlace : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float Direction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blendspace")
		UBlendSpace* StandWalkBlendSpace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blendspace")
		UBlendSpace* CrouchWalkBlendSpace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blendspace")
		UBlendSpace* StandJogBlendSpace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blendspace")
		UBlendSpace* CrouchJogBlendSpace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blendspace")
		UBlendSpace* ProneMoveBlendSpace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blendspace")
		uint8 CanEnterSprint : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blendspace")
		uint8 CanStopSprint : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float Yaw;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float Pitch;

	/** is this character is landed ,default is landed ,which is 1.0f */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float CustomNotIsFallingAlpha;

	/** how much to apply additive base on ads time alpha value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float JogPlayRate;

	/** how much to apply additive base on ads time alpha value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float WalkPlayRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stanima")
		float WalkToStopAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stanima")
		float SprintToWalkAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		float LeftHandIkAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		float RightHandIkAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blend")
		float HorizontalSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blend")
		float VerticalSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FVector WeaponIKRootOffSetEffector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
		FVector BaseHandIKEffector;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="IKControl")
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
		float ADSAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
	    float MaxWeaponSwayPitch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float MaxWeaponSwayYaw;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float WeaponSwayRecoverySpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSway")
		float WeaponSwayScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
		EWeaponBasePoseType CurrentWeaponBaseType;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Stance")
	   ECharacterStance CurrentStance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADS")
		struct FAimAnim AimAnimdata;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turn")
		uint8 TPShouldTurnLeft90 : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turn")
		uint8 TPShouldTurnRight90 : 1;

	protected:
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimDelegate")
			uint8 bWeaponAnimDelegateBindingFinished : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "INSCharacter|Debug")
		uint8 bShowDebugTrace : 1;
#endif

protected:
	UPROPERTY()
		AINSCharacter* OwnerPlayerCharacter;

	UPROPERTY()
		AINSPlayerController* OwnerPlayerController;

	UPROPERTY()
		UCharacterMovementComponent* CharacterMovementComponent;

	UPROPERTY()
		class AINSWeaponBase* CurrentWeaponRef;

	UPROPERTY()
		class UINSWeaponAssets* CurrentWeaponAsstetsRef;

	FScriptDelegate StartWeaponIdleDelegate;

	FScriptDelegate OutWeaponIdleDelegate;

	FScriptDelegate StartMoveDelegate;

	FScriptDelegate StartSprintDelegate;


public:

	virtual void SetIsFalling(bool bIsFallingNow) { this->bIsFalling = bIsFallingNow; }

	virtual void SetWeaponBasePoseType(EWeaponBasePoseType NewBasePoseType) { this->CurrentWeaponBaseType = NewBasePoseType; }

	virtual bool GetIsFalling()const { return bIsFalling; };

	virtual void SetIsSprinting(bool bIsSprintingNow) { this->bIsSprinting = bIsSprintingNow; }

	virtual void SetCanSprint(bool bCanSprintNow) { this->bCanSprint = bCanSprintNow; }

	virtual void SetCanCrouch(bool bCanCrouchNow) { this->bCanCrouch = bCanCrouchNow; }

	virtual void SetIsCrouching(bool bIsCrouchingNow) { this->bIsCrouching = bIsCrouchingNow; }

	virtual void SetIsArmed(bool NewAremdState) { this->bIsArmed = NewAremdState; }

	virtual void SetIsTuring(bool NewState) { this->bIsTurning = NewState; }

	virtual bool GetIsTurning()const { return bIsTurning; }

	virtual void SetIsAiming(bool IsAiming);

	virtual bool GetIsAiming()const { return bIsAiming; }

	virtual void UpdateWalkToStopAlpha();

	virtual void UpdateSprintToWalkAlpha();

	virtual void UpdateTurnConditions();

	virtual void UpdateADSAlpha(float DeltaTimeSeconds);

	virtual void SetStartJump(bool NewJumpState) { this->bStartJump = NewJumpState; }

	virtual void SetCurrentWeaponRef(class AINSWeaponBase* NewWeaponRef);

	virtual void SetViewMode(EViewMode NewViewMode) { CurrentViewMode = NewViewMode; }

	virtual void SetCurrentStance(ECharacterStance NewStance) { CurrentStance = NewStance; }

	virtual void PlayWeaponStartEquipAnim(bool bHasForeGrip)override;

	virtual void BindWeaponAnimDelegate() override;

	virtual void UnbindWeaponAnimDelegate() override;


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

	virtual void SetRotationOffset(FRotator OffsetValue) { this->DeltaRotation = OffsetValue; }

	virtual bool IsFPPlayingWeaponIdleAnim();

	UFUNCTION()
	virtual void StopFPPlayingWeaponIdleAnim();

	UFUNCTION()
	virtual void FPPlayMoveAnimation();

	UFUNCTION()
	virtual void StopFPPlayMoveAnimation();

	virtual void UpdateDirection();

	virtual void UpdateStandStopMoveAlpha();

	virtual void UpdatePitchAndYaw();

	virtual void UpdateHandsIk();

	virtual void UpdatePredictFallingToLandAlpha();

	virtual void UpdateWeaponIkSwayRotation(float deltaSeconds);

	//~ begin INSWeaponAnim Interface

	/** play animation when firing, additive animations are shared between FP and TP */
	virtual void PlayFireAnim(bool bHasForeGrip,bool bIsDry)override;

	virtual void PlayReloadAnim(bool bHasForeGrip, bool bIsDry)override;

	virtual void PlaySwitchFireModeAnim(bool bHasForeGrip)override;

	virtual void PlayWeaponBasePose(bool bHasForeGrip)override;

	virtual void PlayAimAnim()override;

	virtual void PlayStopAimAnim()override;

	virtual void PlaySprintAnim()override;

	virtual void StopPlaySprintAnim()override;

	virtual void OnWeaponAnimDelegateBindingFinished()override;

	UFUNCTION()
	virtual void FPPlayWeaponIdleAnim();

	virtual void PlayWeaponIdleAnim()override;

	//~ end INSWeaponAnim Interface

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	virtual void AddScreenAminDebugMessage(const UAnimMontage* const Anim);
#endif

};
