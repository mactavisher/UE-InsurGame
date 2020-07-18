// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "INSAnimNotify_WeaponReady.generated.h"

/**
 *
 */
UCLASS()
class INSURGENCY_API UINSAnimNotify_WeaponReady : public UAnimNotify
{
	GENERATED_BODY()
public:
	UINSAnimNotify_WeaponReady();

	// Begin UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	// End UAnimNotify interface
};
