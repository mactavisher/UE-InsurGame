// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSDamageModifier/INSDamageModifierBase.h"
#include "INSDamageModifier_Team.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class INSURGENCY_API UINSDamageModifier_Team : public UINSDamageModifierBase
{
	GENERATED_BODY()

protected:
	virtual void ModifyDamage(float& InDamage, FDamageEvent& DamageEvent, AController* Instigator, class AController* Victim)override;
	
};
