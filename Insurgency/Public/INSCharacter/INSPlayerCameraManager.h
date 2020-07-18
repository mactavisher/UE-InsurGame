// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "INSPlayerCameraManager.generated.h"

/**
 *
 */
UCLASS()
class INSURGENCY_API AINSPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()

protected:
    UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="FOVConfig")
	    float ADSFov;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FOVConfig")
		uint8 bWantsToADS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FOVConfig")
		uint8 FOVBlendTime;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="FOVConfig")
	    float FOVBlendSpeed;


protected:
	virtual void LimitViewPitch(FRotator& ViewRotation, float InViewPitchMin, float InViewPitchMax)override;

	virtual void UpdateCamera(float DeltaTime)override;
public:
	virtual void OnAim(float AimTime);

	virtual void OnStopAim(float AimTime);
};
