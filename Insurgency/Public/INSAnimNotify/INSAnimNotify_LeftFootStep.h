// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "INSAnimNotify_LeftFootStep.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSAnimNotify_LeftFootStep : public UAnimNotify
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="FootSocketName")
	FName FootSocketName;

#if WITH_EDITORONLY_DATA
	uint8 bShowDebugInfo : 1;
#endif

	UINSAnimNotify_LeftFootStep();

	// Begin UAnimNotify interface
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	// End UAnimNotify interface

	// effect  to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
    TSubclassOf<class AINSImpactEffect> FootImpactEffectClass;
};
