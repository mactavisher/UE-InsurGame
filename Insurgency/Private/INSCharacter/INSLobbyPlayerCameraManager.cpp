// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSLobbyPlayerCameraManager.h"

AINSLobbyPlayerCameraManager::AINSLobbyPlayerCameraManager(const FObjectInitializer&ObjectInitializer) :Super(ObjectInitializer)
{
	DefaultFOV = 80.f;
	LockedFOV = DefaultFOV;
}

void AINSLobbyPlayerCameraManager::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
	
	Super::ApplyWorldOffset(InOffset, bWorldShift);
}

void AINSLobbyPlayerCameraManager::BeginPlay()
{
	Super::BeginPlay();
}
