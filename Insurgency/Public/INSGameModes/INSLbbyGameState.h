// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "INSLbbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API AINSLbbyGameState : public AGameState
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Audio")
	USoundBase* LobbyBGM;

	virtual void BeginPlay() override;

	virtual void PlayLobbyBGM();
};
