// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSPlayerController.h"
#include "Components/InputComponent.h"
#include "INSCharacter/INSPlayerCameraManager.h"
#include "Engine/Engine.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSGameModes/INSGameModeBase.h"
#include "INSGameModes/INSGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpectatorPawn.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "TimerManager.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "INSItems/INSPickups/INSPickup_Weapon.h"
DEFINE_LOG_CATEGORY(LogAINSPlayerController);

AINSPlayerController::AINSPlayerController(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AINSPlayerCameraManager::StaticClass();
	bPlayerFiring = false;
}

void AINSPlayerController::OnUnPossess()
{

}

void AINSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UE_LOG(LogAINSPlayerController, Log, TEXT("Setting up and bind Player inputs"));
	InputComponent->BindAxis("Turn", this, &AINSPlayerController::AddYawInput);
	InputComponent->BindAxis("LookUp", this, &AINSPlayerController::AddPitchInput);
	InputComponent->BindAxis("MoveForward", this, &AINSPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AINSPlayerController::MoveRight);
	InputComponent->BindAction("ReloadWeapon", IE_Pressed, this, &AINSPlayerController::ReloadWeapon);
	InputComponent->BindAction("Aim", IE_Pressed, this, &AINSPlayerController::AimWeapon);
	InputComponent->BindAction("Aim", IE_Released, this, &AINSPlayerController::StopAimWeapon);
	InputComponent->BindAction("Fire", IE_Pressed, this, &AINSPlayerController::Fire);
	InputComponent->BindAction("Fire", IE_Released, this, &AINSPlayerController::StopFire);
	InputComponent->BindAction("SwitchFireMode", IE_Pressed, this, &AINSPlayerController::SwitchFireMode);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AINSPlayerController::Crouch);
	InputComponent->BindAction("Crouch", IE_Released, this, &AINSPlayerController::UnCrouch);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &AINSPlayerController::Sprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &AINSPlayerController::StopSprint);
	UE_LOG(LogAINSPlayerController, Log, TEXT("finish Setting up and bind Player inputs"));
	//InputComponent->BindAction("Fire", IE_Pressed, this, &AINSPlayerController::Fire);
}

void AINSPlayerController::MoveRight(float Value)
{
	const AINSGameStateBase* GameState = GetWorld()->GetGameState<AINSGameStateBase>();
	if (GameState&&GameState->GetAllowMove())
	{
		if (Value != 0.0f)
		{
			if (GetINSPlayerCharacter())
			{
				GetINSPlayerCharacter()->HandleMoveRightRequest(Value);
			}
			else
			{
				UE_LOG(LogAINSPlayerController, Warning, TEXT("trying to add input on an character that not exist!!!"))
			}
		}
	}
}


void AINSPlayerController::MoveForward(float Value)
{
	const AINSGameStateBase* GameState = GetWorld()->GetGameState<AINSGameStateBase>();
	if (GameState&&GameState->GetAllowMove() && Value != 0.f)
	{

		if (GetINSPlayerCharacter())
		{
			GetINSPlayerCharacter()->HandleMoveForwardRequest(Value);
		}
		else
		{
			UE_LOG(LogAINSPlayerController, Warning, TEXT("trying to add input on an character that not exist!!!"))
		}
	}
}

void AINSPlayerController::PlayerScore(int32 Score)
{
	AINSPlayerStateBase* const CurrentPlayerState = CastChecked<AINSPlayerStateBase>(PlayerState);
	if (CurrentPlayerState)
	{
		CurrentPlayerState->Score = CurrentPlayerState->Score + Score;
	}
}

void AINSPlayerController::ServerMoveRight_Implementation(float Value)
{
	MoveRight(Value);
}

bool AINSPlayerController::ServerMoveRight_Validate(float Value)
{
	return true;
}

void AINSPlayerController::ServerMoveForward_Implementation(float Value)
{
	MoveForward(Value);
}

bool AINSPlayerController::ServerMoveForward_Validate(float Value)
{
	return true;
}

void AINSPlayerController::Crouch()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetINSPlayerCharacter())
		{
			GetINSPlayerCharacter()->HandleCrouchRequest();
		}
	}
	else 
	{
		ServerCrouch();
	}
}

void AINSPlayerController::ServerCrouch_Implementation()
{
	Crouch();
}

bool AINSPlayerController::ServerCrouch_Validate()
{
	return true;
}

void AINSPlayerController::UnCrouch()
{

}

void AINSPlayerController::ServerUnCrouch_Implementation()
{
	UnCrouch();
}

bool AINSPlayerController::ServerUnCrouch_Validate()
{
	return true;
}

void AINSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AINSPlayerController::AimWeapon()
{
	if (GetINSPlayerCharacter() && GetINSPlayerCharacter()->GetCurrentWeapon())
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			GetINSPlayerCharacter()->HandleAimWeaponRequest();
			StopSprint();
		}
		else
		{
			ServerAimWeapon();
		}
	}
}

void AINSPlayerController::StopAimWeapon()
{
	if (GetINSPlayerCharacter() && GetINSPlayerCharacter()->GetCurrentWeapon())
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			GetINSPlayerCharacter()->HandleStopAimWeaponRequest();
		}
		else
		{
			ServerStopAimWeapon();
		}
	}
}

void AINSPlayerController::ServerAimWeapon_Implementation()
{
	AimWeapon();
}

bool AINSPlayerController::ServerAimWeapon_Validate()
{
	return true;
}

void AINSPlayerController::ServerStopAimWeapon_Implementation()
{
	StopAimWeapon();
}

bool AINSPlayerController::ServerStopAimWeapon_Validate()
{
	return true;
}

void AINSPlayerController::ReloadWeapon()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetINSPlayerCharacter())
		{
			StopAimWeapon();
			GetINSPlayerCharacter()->HandleWeaponRealoadRequest();
		}
	}
	else
	{
		ServerStopAimWeapon();
		ServerReloadWeapon();
	}
}

void AINSPlayerController::Fire()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		const AINSGameModeBase* const CurrentGameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();

		if (GetINSPlayerCharacter() && CurrentGameMode&&CurrentGameMode->GetIsAllowFire())
		{
			GetINSPlayerCharacter()->HandleFireRequest();
		}
	}
	else
	{
		ServerFire();
		bPlayerFiring = true;
	}
}

void AINSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AINSPlayerController::AddPitchInput(float Val)
{
	Super::AddPitchInput(Val);
	LastPlayerInputRot = RotationInput;
}

void AINSPlayerController::AddYawInput(float Val)
{
	Super::AddYawInput(Val);
	LastPlayerInputRot = RotationInput;
}

void AINSPlayerController::ServerFire_Implementation()
{
	Fire();
}

bool AINSPlayerController::ServerFire_Validate()
{
	return true;
}

void AINSPlayerController::StopFire()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetINSPlayerCharacter())
		{
			GetINSPlayerCharacter()->HandleStopFireRequest();
		}
	}
	else
	{
		ServerStopFire();
		bPlayerFiring = false;
	}
}

void AINSPlayerController::Sprint()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		const AINSGameStateBase* GameState = GetWorld()->GetGameState<AINSGameStateBase>();
		if (GetINSPlayerCharacter() && GameState->GetAllowMove())
		{
			GetINSPlayerCharacter()->HandleStartSprintRequest();
		}
	}
	else
	{
		ServerSprint();
	}
}

void AINSPlayerController::ServerSprint_Implementation()
{
	Sprint();
}

bool AINSPlayerController::ServerSprint_Validate()
{
	return true;
}

void AINSPlayerController::StopSprint()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetINSPlayerCharacter())
		{
			GetINSPlayerCharacter()->HandleStopSprintRequest();
		}
	}
	else
	{
		ServerStopSprint();
	}
}

void AINSPlayerController::ServerStopSprint_Implementation()
{
	StopSprint();
}

bool AINSPlayerController::ServerStopSprint_Validate()
{
	return true;
}

void AINSPlayerController::ServerStopFire_Implementation()
{
	StopFire();
}

bool AINSPlayerController::ServerStopFire_Validate()
{
	return true;
}

void AINSPlayerController::ServerReloadWeapon_Implementation()
{
	ReloadWeapon();
}

bool AINSPlayerController::ServerReloadWeapon_Validate()
{
	return true;
}

void AINSPlayerController::ServerFinishReloadWeapon_Implementation()
{
	if (PossessedINSCharacter)
	{
		if (PossessedINSCharacter->GetCurrentWeapon())
		{
			PossessedINSCharacter->GetCurrentWeapon()->FinishReloadWeapon();
		}
	}
}

bool AINSPlayerController::ServerFinishReloadWeapon_Validate()
{
	return true;
}

void AINSPlayerController::SwitchFireMode()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetINSPlayerCharacter())
		{
			GetINSPlayerCharacter()->HandleSwitchFireModeRequest();
		}
	}
	else
	{
		ServerSwitchFireMode();
	}
}


void AINSPlayerController::InspecWeapon()
{

}

void AINSPlayerController::ServerInspectWeapon_Implementation()
{
	if (GetLocalRole() == ROLE_Authority) {

	}
	else
	{
		InspecWeapon();
	}
}

bool AINSPlayerController::ServerInspectWeapon_Validate()
{
	return true;
}

void AINSPlayerController::PickupWeapon(class AINSPickup_Weapon* NewWeaponPickup)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		NewWeaponPickup->SetClaimedPlayer(this);
		UClass* ActualWeaponClassFromPickup = NewWeaponPickup->GetActualWeaponClass();
		if (ActualWeaponClassFromPickup)
		{
			FActorSpawnParameters NewWeaponSpawnParams;
			NewWeaponSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			FTransform NewWeaponSpawnTransform;
			NewWeaponSpawnTransform.SetLocation(GetINSPlayerCharacter()->GetActorLocation());
			NewWeaponSpawnTransform.SetRotation(GetINSPlayerCharacter()->GetActorRotation().Quaternion());
			NewWeaponSpawnTransform.SetScale3D(FVector::OneVector);
			class AINSWeaponBase* SpawnedNewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(ActualWeaponClassFromPickup, NewWeaponSpawnTransform, this);
			if (SpawnedNewWeapon)
			{
				EquipWeapon(SpawnedNewWeapon);
			}
		}
	}
	else
	{
		ServerPickupWeapon(NewWeaponPickup);
	}
}

void AINSPlayerController::ServerPickupWeapon_Implementation(class AINSPickup_Weapon* NewWeaponPickup)
{
	PickupWeapon(NewWeaponPickup);
}

bool AINSPlayerController::ServerPickupWeapon_Validate(class AINSPickup_Weapon* NewWeaponPickup)
{
	return true;
}

void AINSPlayerController::ServerSwitchFireMode_Implementation()
{
	SwitchFireMode();
}

bool AINSPlayerController::ServerSwitchFireMode_Validate()
{
	return true;
}

void AINSPlayerController::EquipWeapon(class AINSWeaponBase* NewWeaponToEquip)
{
	if (!NewWeaponToEquip)
	{
		NewWeaponToEquip = GetDefaultWeapon();
	}
	if (NewWeaponToEquip)
	{
		GetINSPlayerCharacter()->SetCurrentWeapon(NewWeaponToEquip);
		//GetINSPlayerCharacter()->HandleEquipWeaponRequest();
	}
}

void AINSPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AINSPlayerController::ServerEquipWeapon_Implementation(class AINSWeaponBase* NewWeaponToEquip)
{
	EquipWeapon(NewWeaponToEquip);
}

bool AINSPlayerController::ServerEquipWeapon_Validate(class AINSWeaponBase* NewWeaponToEquip)
{
	return true;
}

void AINSPlayerController::ClientShowThreaten_Implementation()
{

}

bool AINSPlayerController::ClientShowThreaten_Validate()
{
	return true;
}

AINSPlayerCharacter* AINSPlayerController::GetINSPlayerCharacter()
{
	if (PossessedINSCharacter)
	{
		return PossessedINSCharacter;
	}
	else if (GetPawn())
	{
		return Cast<AINSPlayerCharacter>(GetPawn());
	}
	return nullptr;
}

class AINSWeaponBase* AINSPlayerController::GetDefaultWeapon()
{
	const AINSGameModeBase* CurrentGameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
	if (CurrentGameMode)
	{
		PlayerDefaultWeaponClass = CurrentGameMode->GetRandomWeapon();
		DefaultWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(PlayerDefaultWeaponClass, GetINSPlayerCharacter()->GetActorTransform(), this, GetINSPlayerCharacter(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (DefaultWeapon)
		{
			DefaultWeapon->SetWeaponState(EWeaponState::NONE);
			UGameplayStatics::FinishSpawningActor(DefaultWeapon, GetINSPlayerCharacter()->GetActorTransform());
		}
		return DefaultWeapon;
	}
	return nullptr;
}

bool AINSPlayerController::IsEnemyFor(class AActor* Other)
{
	if (nullptr == Other)
	{
		return false;
	}
	else
	{
		bool bIsOtherAEnemy = false;
		const UClass* const OtherActorClass = Other->GetClass();
		UE_LOG(LogAINSPlayerController, Warning, TEXT("Player %s has see something ,other actor class:%s"), *this->GetName(), *Other->GetName())
			if (OtherActorClass->IsChildOf(AINSPlayerCharacter::StaticClass()))
			{
				const AINSPlayerCharacter* const OtherPlayerCharacter = CastChecked<AINSPlayerCharacter>(Other);
				const AINSPlayerStateBase* const OtherPlayerState = OtherPlayerCharacter->GetPlayerState() == nullptr ? nullptr : OtherPlayerCharacter->GetPlayerStateChecked<AINSPlayerStateBase>();
				const AINSPlayerStateBase* const MyPlayerState = GetINSPlayerCharacter()->GetPlayerState() == nullptr ? nullptr : GetINSPlayerCharacter()->GetPlayerStateChecked<AINSPlayerStateBase>();
				if (OtherPlayerState&&MyPlayerState)
				{
					const AINSTeamInfo* const OtherPlayerCharacterTeam = OtherPlayerState->GetPlayerTeam();
					const AINSTeamInfo* const MyPlayerCharacterTeam = MyPlayerState->GetPlayerTeam();
					if (OtherPlayerCharacterTeam&&MyPlayerCharacterTeam)
					{
						const ETeamType OtherPlayerTeamType = OtherPlayerCharacterTeam->GetTeamType();
						const ETeamType MyTeamType = MyPlayerCharacterTeam->GetTeamType();
						if (OtherPlayerTeamType != MyTeamType)
						{
							bIsOtherAEnemy = true;
						}
					}
				}
			}
		return bIsOtherAEnemy;
	}
}

bool AINSPlayerController::HasSeeEnemy()
{
	return IsEnemyFor(GetViewTarget());
}

void AINSPlayerController::OnCharacterDeath()
{
	//const AINSGameStateBase* const CurrentGameState = GetWorld()->GetGameState<AINSGameStateBase>();
	//const float RespawnTime = CurrentGameState->GetRespawnTime();
	// UnPossess();
	//Possess(GetSpectatorPawn());
	//GetWorldTimerManager().SetTimer(CharacterRespawnTimer, this, &AINSPlayerController::RespawnPlayer, 1.f, false, RespawnTime);
	//StartSpectatingOnly();
	//GetPlayerState<AINSPlayerStateBase>()->bIsSpectator = true;
	//ChangeState(NAME_Spectating);
	//ChangeState(NAME_)
}

AINSTeamInfo*  AINSPlayerController::GetMyTeamInfo()
{
	const AINSPlayerStateBase* const MyPlayerState = GetPlayerState<AINSPlayerStateBase>();
	if (MyPlayerState)
	{
		return MyPlayerState->GetPlayerTeam();
	}
	return nullptr;
}

void AINSPlayerController::ReceiveOverlapPickupItems(class AActor* PickupItems)
{

}

void AINSPlayerController::ReceiveStartRespawn()
{
	RespawnPlayer();
}

void AINSPlayerController::RespawnPlayer()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		AINSGameModeBase* const CurrentGameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
		if (CurrentGameMode)
		{
			CurrentGameMode->RestartPlayer(this);
		}
	}
}

void AINSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (GetPawn()->GetClass()->IsChildOf(AINSPlayerCharacter::StaticClass()))
	{
		PossessedINSCharacter = CastChecked<AINSPlayerCharacter>(InPawn);
		PossessedINSCharacter->SetPlayerState(CastChecked<AINSPlayerStateBase>(PlayerState));
		if (GetLocalRole() == ROLE_Authority)
		{
			EquipWeapon(GetDefaultWeapon());
		}
		else
		{
			ServerEquipWeapon(GetDefaultWeapon());
		}
	}
	else if (GetPawn()->GetClass()->IsChildOf(ASpectatorPawn::StaticClass()))
	{
		UE_LOG(LogAINSPlayerController, Log, TEXT("Player Character is respawning,controll specatator pawn for now"));
	}
}
