// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "INSCharacterMovementComponent.generated.h"

class AINSPlayerCharacter;

/**
 *
 */
UCLASS()
class INSURGENCY_API UINSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()
protected:

	/** speed modifier when sprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "StanimaModifier")
		float SprintSpeedModifier;

	/** speed modifier when crouch */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "StanimaModifier")
		float CrouchSpeedModifier;

	/** speed modifier when prone */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "StanimaModifier")
		float ProneSpeedModifier;

	/** speed modifier when aim */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "StanimaModifier")
		float AimSpeedModifier;

	/** cache stand walk speed as base speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "StanimaModifier")
		float BaseWalkSpeed;

	/** cache this speed value for later calculate aiming speed purposes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StanimaModifier")
		float SpeedBeforeAim;

	/** accumulated idle time */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StanimaModifier")
		float AccumulatedIdleTime;

	/** how much time this player can enter idle state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StanimaModifier")
		float IdleStateTime;

	/** how much time this player can enter bored state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StanimaModifier")
		float BoredStateTime;

	UPROPERTY()
	uint8 bInIdleState:1;

	UPROPERTY()
	uint8 bInBoredState : 1;

protected:
	AINSCharacter* INSCharacterOwner;

protected:
	//~ begin UCharacterMovement Interface
	virtual void BeginPlay()override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;
	//~ end UCharacterMovement Interface
public:

	virtual void StartCrouch();

	virtual void EndCrouch();

	virtual void StartSprint();

	virtual void EndSprint();

	virtual void StartProne();

	virtual void  EndProne();

	virtual void StartAim();

	virtual void EndAim();

	virtual void CheckCharacterIdleState(const float DeltaTime);

	inline virtual float GetSprintSpeed()const { return BaseWalkSpeed * SprintSpeedModifier; }

	inline virtual float GetCrouchSpeed()const { return BaseWalkSpeed * CrouchSpeedModifier; }

	inline virtual float GetAimSpeed()const { return SpeedBeforeAim * AimSpeedModifier; }

	inline virtual float GetAcculatedIdleTime()const { return AccumulatedIdleTime; }
};
