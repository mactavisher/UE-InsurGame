// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "INSLobbyPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API AINSLobbyPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()
protected:
	virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
	virtual void BeginPlay() override;
};
