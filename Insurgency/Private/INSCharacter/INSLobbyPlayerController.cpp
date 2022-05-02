// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSLobbyPlayerController.h"

#include "Components/AudioComponent.h"
#include "INSCharacter/INSLobbyPlayerCameraManager.h"

// AINSLobbyPlayController::AINSLobbyPlayController(const FObjectInitializer&ObjectInitializer) :Super(ObjectInitializer)
// {
// 	
// }

AINSLobbyPlayerController::AINSLobbyPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AudioComponent = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this,TEXT("AudioComponent"));
	AudioComponent->SetUISound(true);
	PlayerCameraManagerClass = AINSLobbyPlayerCameraManager::StaticClass();
}

void AINSLobbyPlayerController::BeginPlay()
{
	INSGameInstance = GetWorld()->GetGameInstance<UINSGameInstance>();
	Super::BeginPlay();
	AudioComponent->OnAudioFinished.AddDynamic(this, &AINSLobbyPlayerController::OnLobbyBgmPlayFinished);
	DisableInput(this);
	SetShowMouseCursor(true);
	if (IsNetMode(NM_DedicatedServer))
	{
		AudioComponent->DestroyComponent();
		AudioComponent = nullptr;
	}
	PlayLobbyBGM();
}

void AINSLobbyPlayerController::PlayLobbyBGM()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	if (LobbyBMG)
	{
		AudioComponent->SetSound(LobbyBMG);
		AudioComponent->Play();
	}
}

void AINSLobbyPlayerController::OnLobbyBgmPlayFinished()
{
	AudioComponent->SetSound(LobbyBMG);
	AudioComponent->Play();
}

void AINSLobbyPlayerController::GetAvailableWeaponList(TArray<FWeaponInfoData>& Weapons)
{
}

void AINSLobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AINSLobbyPlayerController::OnClickItemIcon(int32 ItemId)
{
}
