// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "INSDamageModifierBase.generated.h"


class AINSGameModeBase;

/**
 *  damage modifier base class,provides a functionality to modify the original damage
 *  that a pawn or player takes
 */
UCLASS(Abstract,NotBlueprintable)
class INSURGENCY_API UINSDamageModifierBase : public UObject
{
	GENERATED_BODY()
	friend class AINSGameModeBase;
	/** damage modifier function,all subclasses must implement this function*/
	virtual void ModifyDamage(float& OutDamage,FDamageEvent& DamageEvent,AController* Instigator,class AController* Victim)PURE_VIRTUAL(void);
};
