// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSGameModes/INSGameModeBase.h"
#include "INSGameMode_Free_For_All.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API AINSGameMode_Free_For_All : public AINSGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:
	virtual bool GetIsTeamDamage(class AController* DamageInstigator, class AController* Victim) override;
};
