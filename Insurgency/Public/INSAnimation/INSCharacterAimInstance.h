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
class UINSCharacterMovementComponent;
class UINSStaticAnimData;
class AINSWeaponBase;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSCharacterAimInstance, Log, All);

/**
 *
 */
UCLASS(Blueprintable)
class INSURGENCY_API UINSCharacterAimInstance : public UAnimInstance
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
	uint8 bCanEnterSprint : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
	float Yaw;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
	float Pitch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "State")
	float bIdleState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "State")
	float bBoredState;

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

	/** base hands IK effector ,relative to origin hands position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
	FVector BaseHandIKEffector;

	/** ads hands IK effector ,relative to origin hands position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
	FVector ADSHandIKEffector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Procedure")
	FRotator WeaponIKSwayRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Procedure")
	FVector WeaponSwayLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
	EViewMode CurrentViewMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
	float Direction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
	EWeaponBasePoseType CurrentWeaponBaseType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
	EWeaponReloadType CurrentWeaponReloadType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
	float LastBoredAnimPlayTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControl")
	float BoredAnimPlayTimeInterval;

	/** target x scale when aim ,relative to origin hands position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="IK")
	float AimHandIKXLocationValue;

	/** current interpolated x scale when aim ,relative to origin hands position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IK")
	float CurrentAimHandIKXLocationValue;

	UPROPERTY()
	uint8 bInitialized:1;

	UPROPERTY()
	uint8 bValidPlayAnim:1;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimDelegate")
	uint8 bWeaponAnimDelegateBindingFinished : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "INSCharacter|Debug")
	uint8 bShowDebugTrace : 1;
#endif

	/** timer for bored animation play */
	FTimerHandle BoredAnimPlayTimer;

protected:
	/** cached player character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UINSCharacterMovementComponent* CharacterMovementComponent;

	/** cached weapon that currently in use */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AINSWeaponBase* CurrentWeapon;

	/** cached player Animation data*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UINSStaticAnimData* CurrentWeaponAnimData;

	/** cached player character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AINSCharacter* OwnerPlayerCharacter;


public:
	/**
	 * @Desc  sets the current base pose type
	 * @Param NewBasePoseType target base pose type
	 */
	virtual void SetWeaponBasePoseType(EWeaponBasePoseType NewBasePoseType);

	/** returns if in falling state */
	virtual bool GetIsFalling() const { return bIsFalling; };

	/**
	 * @Desc    sets sprint status
	 * @Param   target base pose type
	 */
	virtual void SetIsSprinting(const bool NewSprintState);

	virtual void SetSprintPressed(bool NewSprintPressed);

	virtual void UpdateCanEnterSprint();

	virtual void SetIsCrouching(bool bIsCrouchingNow) { this->bIsCrouching = bIsCrouchingNow; }

	virtual void SetIsArmed(bool NewArmedState) { this->bIsArmed = NewArmedState; }

	virtual void SetIsAiming(bool IsAiming);

	virtual bool GetIsAiming() const { return bIsAiming; }

	UFUNCTION()
	virtual void PlayBoredAnim();

	virtual void SetStartJump(bool NewJumpState) { this->bStartJump = NewJumpState; }

	/**
	 * Set the currently used weapon and weapon animation data
	 * @Param NewWeapon   AINSWeaponBase
	 */
	virtual void SetCurrentWeapon(class AINSWeaponBase* NewWeapon);

	virtual void SetCurrentWeaponAnimData(UINSStaticAnimData* NewAnimData);

	/**
	 * Set the currently view mode could either be FPS or TPS
	 * @Param NewViewMode   EViewMode
	 */
	virtual void SetViewMode(EViewMode NewViewMode) { CurrentViewMode = NewViewMode; }


protected:
	/** native update for variables tick */
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void NativeBeginPlay() override;

	/** calculate horizontal speed */
	virtual void UpdateHorizontalSpeed();

	/** calculate horizontal speed */
	virtual void UpdateVerticalSpeed();

	virtual bool IsFPPlayingWeaponIdleAnim();

	virtual void UpdateDirection();

	virtual void UpdatePitchAndYaw();

	virtual void UpdateIsAiming();

	virtual void UpdateWeaponBasePoseType();

	virtual void UpdateHandsIk();

	virtual void UpdateSprintPlaySpeed();

	virtual void UpdateIsMoving();

	virtual void UpdateIsFalling();

	virtual bool CheckValid();

	//~ begin INSWeaponAnim Interface
public:
	virtual bool GetIsValidPlayAnim() const { return bValidPlayAnim; }
	virtual void OnCharacterJustLanded();
	virtual void SetIdleState(bool NewIdleState);
	virtual void SetBoredState(bool NewBoredState);
	virtual void SetBaseHandsIkLocation(const FVector NewLocation);
	virtual void SetAimHandIKXLocation(const float NewVal) { AimHandIKXLocationValue = NewVal; }
	virtual void SetWeaponReloadType(EWeaponReloadType NewReloadType) { CurrentWeaponReloadType = NewReloadType; }
	UFUNCTION()
	virtual void StopFPPlayingWeaponIdleAnim();
	UFUNCTION()
	virtual void FPPlayMoveAnimation();
	UFUNCTION()
	virtual void StopFPPlayMoveAnimation();
	UFUNCTION()
	virtual float FPPlayWeaponIdleAnim();
	//~ end INSWeaponAnim Interface

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	virtual void AddScreenAminDebugMessage(const UAnimMontage* const Anim);
#endif

	virtual bool GetIsAnimInitialized() const { return bInitialized; }
	virtual float PlayWeaponBasePose() { return 0.f; };
	virtual float PlayWeaponEquipAnim() { return 0.f; };
	virtual float PlayWeaponReloadAnim(bool bDryReload) { return 0.f; };
	virtual float PlayWeaponSwitchFireModeAnim() { return 0.f; };
	virtual float PlayWeaponUnEquipAnim() { return 0.f; };
	virtual float PlayFireAnim() { return 0.f; }
	virtual float PlayPullTriggerAnim() { return 0.f; }
	virtual float PlayIdleAnim() { return 0.f; }
};
