// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameModes/INSLbbyGameState.h"

#include "Kismet/GameplayStatics.h"

void AINSLbbyGameState::BeginPlay()
{
	Super::BeginPlay();
	PlayLobbyBGM();
}

void AINSLbbyGameState::PlayLobbyBGM()
{
	UGameplayStatics::PlaySound2D(GetWorld(), LobbyBGM);
}
