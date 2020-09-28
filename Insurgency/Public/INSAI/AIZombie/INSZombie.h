// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSCharacter/INSCharacter.h"
#include "INSZombie.generated.h"

class UBehaviorTree;

/**
 * 
 */
UCLASS()
class INSURGENCY_API AINSZombie : public AINSCharacter
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="ZombieBehavior")
		UBehaviorTree* ZombiebehaviorTree;

public:
	FORCEINLINE class UBehaviorTree* GetZombieBehaviorTree()const { return ZombiebehaviorTree; }
};
