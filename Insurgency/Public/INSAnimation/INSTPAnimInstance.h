// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSAnimation/INSCharacterAimInstance.h"
#include "INSTPAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSTPAnimInstance : public UINSCharacterAimInstance
{
	GENERATED_UCLASS_BODY()

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
		float StandToJogAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
	    uint8 bCanEnterJog:1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|Transform")
		uint8 bCanTurnInPlace : 1;

	UPROPERTY()
	uint8 bCanEnterJogFromSprint:1;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turn")
		uint8 TPShouldTurnLeft90 : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turn")
		uint8 TPShouldTurnRight90 : 1;

	/** how much to apply additive base on ads time alpha value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float JogPlayRate;

	/** how much to apply additive base on ads time alpha value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float JogSpeed;

	/** how much to apply additive base on ads time alpha value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BlendSpace")
		float WalkPlayRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stanima")
		float WalkToStopAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stanima")
		float SprintToWalkAlpha;

	/**
	 * @Desc tick variables that drives animation playing
	 * 
	 * @Params DeltaSeconds Tick interval
	 */
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

	/**
	 * @Desc initialize animation set owner ptr
	 */
	virtual void NativeInitializeAnimation()override;

	/**
	 * @Desc set up current weapon and animation assets
	 * 
	 * @Param NewWeapon Weapon to use in this animation
	 */
	virtual void SetCurrentWeaponAndAnimationData(class AINSWeaponBase* NewWeapon)override;

	/**
	 * used for entry state for jog
	 */
	virtual void UpdateCanEnterJogCondition();

	/**
	 * @desc update play turn in place animation conditions
	 */
	virtual void UpdateTurnConditions();

	/**
	 * @Desc update jog speed ,use for jog blend space blending
	 */
	virtual void UpdateJogSpeed();


	virtual void UpdateCanEnterSprint();

	virtual float PlayReloadAnim(bool bIsDry)override;

	virtual float PlayWeaponBasePose()override;

	virtual float PlayFireAnim()override;

	virtual void UpdateIsAiming();

	virtual void UpdateEnterJogState();

	virtual void SetIsAiming(bool IsAiming)override;

	virtual float PlayWeaponStartEquipAnim()override;

	virtual void UpdatePredictFallingToLandAlpha();

};
