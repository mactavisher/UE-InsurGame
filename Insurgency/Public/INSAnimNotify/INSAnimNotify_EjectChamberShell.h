// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "INSAnimNotify_EjectChamberShell.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSAnimNotify_EjectChamberShell : public UAnimNotify
{
	GENERATED_BODY()
	UINSAnimNotify_EjectChamberShell();
public:
	// Begin UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	// End UAnimNotify interface
};
