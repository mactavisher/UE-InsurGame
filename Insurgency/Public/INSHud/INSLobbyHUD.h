// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "INSCharacter/INSLobbyPlayerController.h"
#include "INSLobbyHUD.generated.h"

class UUserWidget;
class UDataTable;

/**
 * 
 */
UCLASS()
class INSURGENCY_API AINSLobbyHUD : public AHUD
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> LobbyMainWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> LobbyMainMenuWidgetClass;

	UPROPERTY()
	UUserWidget* LobbyMainWidgetInstance;

	UPROPERTY()
	AINSLobbyPlayerController* OwningLobbyController;


	virtual void BeginPlay() override;

	UPROPERTY()
	UDataTable* LobbyMainMenuDataTable;

	virtual void CreateLobbyMainWidget();

	virtual void LoadLobbyMainMenus();
};
