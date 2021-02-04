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

	/** hands shift value when firing to simulate recoil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AnimationMode")
		FVector FiringHandsShift;

	/**
	 * update ads alpha when aim
	 * @Param DeltaSeconds  DeltaSeconds
	 */
	virtual void UpdateAdsAlpha(float DeltaSeconds);


	/**
	 * update Animation Variables
	 * @Param DeltaSeconds  DeltaSeconds
	 */
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

	/**
	 * update HandsShift when firing
	 * @Param DeltaSeconds  DeltaSeconds
	 */
	virtual void UpdateFiringHandsShift(float DeltaSeconds);
};
