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

	virtual void NativeUpdateAnimation(float DeltaSeconds)override;
	virtual void UpdateStandStopMoveAlpha();

	virtual void UpdateCanEnterJogFromSprint();

	virtual void UpdateWalkToStopAlpha();

	virtual void UpdateSprintToWalkAlpha();

	virtual void UpdateTurnConditions();

	virtual void UpdateJogSpeed();
	virtual void UpdateCanEnterSprint();

	virtual void UpdateEnterJogState();

};
