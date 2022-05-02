// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSWeaponCrossHair/INSCrossHairBase.h"
#include "INSCrossHair_SimpleDot.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSCrossHair_SimpleDot : public UINSCrossHairBase
{
	GENERATED_UCLASS_BODY()
public:
	//~ Begin UINSCrossHairBase Interface
	virtual void DrawCrossHair(class UCanvas* InCanvas, class AINSWeaponBase* InWeapon, FLinearColor DrawColor) override;
	//~ End UINSCrossHairBase Interface
};
