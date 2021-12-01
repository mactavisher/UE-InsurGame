// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "INSAssets/INSWeaponAssets.h"
#include "INSCore/INSGameInstance.h"
#include "INSLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API AINSLobbyPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="sound")
	USoundBase* LobbyBMG;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PawnMakeNoiseComp", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* AudioComponent;

	UPROPERTY()
	UINSGameInstance* INSGameInstance;
	
	virtual void BeginPlay() override;

	virtual void PlayLobbyBGM();

	UFUNCTION()
	virtual void OnLobbyBgmPlayFinished();

	virtual void GetAvailableWeaponList(TArray<FWeaponInfoData>& Weapons);

	virtual void SetupInputComponent() override;

	virtual void OnClickItemIcon(int32 ItemId);
};
