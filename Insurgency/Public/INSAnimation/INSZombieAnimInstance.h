// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "INSAI/AIZombie/INSZombie.h"
#include "INSZombieAnimInstance.generated.h"

class UCharacterMovementComponent;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSZombieAnimation, Log, All);

/**
 *  Animation Instance for zombie pawns
 */
UCLASS()
class INSURGENCY_API UINSZombieAnimInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
	uint8 bIsMoving : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
	uint8 bIsMovingHorizontal : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
	uint8 bIsFalling : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFCharacterAnim|State")
	uint8 bIsInAir : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ZombiePawn")
	AINSZombie* ZombiePawnOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ZombiePawn")
	UCharacterMovementComponent* ZombiePawnMovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ZombiePawn")
	EZombieMoveMode ZombieCurrentMoveMode;


protected:
	//~Begin UAnimInstance Interface

	/**
	 * called when AnimInstance initialized in c++ side
	 */
	virtual void NativeInitializeAnimation() override;

	/**
	 * called every frame
	 * @param DeltaSeconds Update interval
	 */
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void EvaluateAnimCurves();
	//~End UAnimInstance Interface

	/** update zombie moving condition*/
	virtual void UpdateIsMoving();

	/** update zombie horizontal moving condition*/
	virtual void UpdateIsMovingHorizontal();

	/**
	 * update zombie falling condition
	 */
	virtual void UpdateIsFalling();

public:
	/**
	 * Get the Owner zombie pawn
	 */
	virtual class AINSZombie* GetZombiePawnOwner() const { return ZombiePawnOwner; }

	/**
	 * Get the owner zombie movement comp
	 */
	virtual class UCharacterMovementComponent* GetZombieMovementComp() const { return ZombiePawnMovementComp; }

	/**
	 * Set the zombie Current MoveMode
	 * @Param NewMoveMode MoveMode to Set
	 */
	virtual void SetZombieMoveMoe(EZombieMoveMode NewMoveMode) { this->ZombieCurrentMoveMode = NewMoveMode; }

	/**
	 * Get the owner zombie movement Mode
	 */
	virtual EZombieMoveMode GetZombieCurrentMoveMode() const { return ZombieCurrentMoveMode; }
};
