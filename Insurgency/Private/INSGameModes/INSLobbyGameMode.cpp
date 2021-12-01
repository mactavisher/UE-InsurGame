// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameModes/INSLobbyGameMode.h"

#include "INSCharacter/INSLobbyCharacter.h"
#include "INSCharacter/INSLobbyPlayerController.h"
#include "INSGameModes/INSLbbyGameState.h"
#include "INSGameModes/INSLobbyGameMode.h"

#include "INSHud/INSLobbyHUD.h"

AINSLobbyGameMode::AINSLobbyGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
	PlayerControllerClass = AINSLobbyPlayerController::StaticClass();
	DefaultPawnClass = AINSLobbyCharacter::StaticClass();
	GameStateClass = AINSLbbyGameState::StaticClass();
	HUDClass = AINSLobbyHUD::StaticClass();
}
