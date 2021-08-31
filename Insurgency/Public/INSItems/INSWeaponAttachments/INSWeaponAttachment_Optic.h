// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSItems/INSWeaponAttachments/INSWeaponAttachment.h"
#include "INSWeaponAttachment_Optic.generated.h"

class USceneCaptureComponent2D;
/**
 * 
 */
UCLASS(Blueprintable)
class INSURGENCY_API AINSWeaponAttachment_Optic : public AINSWeaponAttachment
{
	GENERATED_UCLASS_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "InteractComp", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* OpticMeshComp;

	/** When ads , modify hands IK x value to make scope closer */
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="ADS")
	    float HandIKXLocationValue;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "SceneCaptureComp", meta = (AllowPrivateAccess = "true"))
	   USceneCaptureComponent2D* SceneCaptureComp;

	/** returns the ads alpha */
	virtual float GetADSAlpha();

	virtual void Tick(float DeltaTime)override;

	virtual void PostInitializeComponents()override;

	virtual void AttachToWeaponSlot()override;

	virtual void OnRep_OwnerWeapon()override;

	virtual void BeginPlay()override;



public:
	/** returns the optic mesh comp */
	FORCEINLINE UStaticMeshComponent* GetOpticMeshComp()const { return OpticMeshComp; }

	virtual float GetHandsIKXValue()const { return HandIKXLocationValue; }
	/** returns the Optic sight aligner socket transform */
	virtual FTransform GetOpticSightTransform();
};
