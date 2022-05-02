// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "INSAnimNotify_FinishEquipping.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSAnimNotify_FinishEquipping : public UAnimNotify
{
	GENERATED_BODY()
public:
	UINSAnimNotify_FinishEquipping();

	// Begin UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference) override;
	// End UAnimNotify interface
};
