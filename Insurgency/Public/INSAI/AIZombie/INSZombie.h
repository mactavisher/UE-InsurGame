// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSCharacter/INSCharacter.h"
#include "INSZombie.generated.h"

class UBehaviorTree;

UENUM(BlueprintType)
enum class EZombieMoveMode:uint8
{
  Walk                 UMETA(DisplayName = "Walk"),
  Shamble              UMETA(DisplayName = "Shamble"),
  Chase                UMETA(DisplayName = "Chase"),
};


/**
 * 
 */
UCLASS()
class INSURGENCY_API AINSZombie : public AINSCharacter
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="ZombieBehavior")
		UBehaviorTree* ZombiebehaviorTree;

	    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ZombieMovement")
		EZombieMoveMode CurrentZombieMoveMode;

public:
	FORCEINLINE class UBehaviorTree* GetZombieBehaviorTree()const { return ZombiebehaviorTree; }

	/**
	 * Set the zombie current move mode
	 * @param NewZombieMoveMode  the new zombie move mode to set for this zombie
	 */
	virtual void SetZombieMoveMode(EZombieMoveMode NewZombieMoveMode) { CurrentZombieMoveMode = NewZombieMoveMode; };
};
