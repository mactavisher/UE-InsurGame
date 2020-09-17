// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSPlayerController.h"
#include "Components/InputComponent.h"
#include "INSCharacter/INSPlayerCameraManager.h"
#include "Engine/Engine.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSGameModes/INSGameModeBase.h"
#include "INSGameModes/INSGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpectatorPawn.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Engine/NetDriver.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "TimerManager.h"
#include "INSHud/INSHUDBase.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "INSItems/INSPickups/INSPickup_Weapon.h"
#include "Components/AudioComponent.h"
#ifndef GEngine
#include "Engine/Engine.h"
#endif
DEFINE_LOG_CATEGORY(LogAINSPlayerController);

AINSPlayerController::AINSPlayerController(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AINSPlayerCameraManager::StaticClass();
	bPlayerFiring = false;
	AmbientAudioComp = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("AmbientAudioComp"));
	bAutoReload = true;
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

void AINSPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	INSPlayerState = Cast<AINSPlayerStateBase>(PlayerState);
	if (GetNetMode() == ENetMode::NM_Client)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("controller player State Replicated"));
	}
}

void AINSPlayerController::SetPlayerTeam(class AINSTeamInfo* NewTeam)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayerTeam = NewTeam;
		GetINSPlayerState()->SetPlayerTeam(PlayerTeam);
	}
}

void AINSPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
}

void AINSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
}

void AINSPlayerController::SetCurrentWeapon(class AINSWeaponBase* NewWeapon)
{
	CurrentWeapon = NewWeapon;
	if (CurrentWeapon)
	{
		BindWeaponDelegate();
	}
}

void AINSPlayerController::BindWeaponDelegate()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->OnAmmoLeftEmpty.AddDynamic(this, &AINSPlayerController::OnWeaponAmmoLeftEmpty);
		CurrentWeapon->OnClipLow.AddDynamic(this, &AINSPlayerController::OnWeaponClipAmmoLow);
		CurrentWeapon->OnClipEmpty.AddDynamic(this, &AINSPlayerController::OnWeaponClipEmpty);
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
	DOREPLIFETIME(AINSPlayerController, PlayerTeam);
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
		else if(GetLocalRole()==ROLE_AutonomousProxy)
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

void AINSPlayerController::OnRep_PlayerTeam()
{
	// 	if (GetLocalRole() == ROLE_Authority)
	// 	{
	// 		GetINSPlayerState()->SetPlayerTeam(PlayerTeam);
	// 		GetINSPlayerCharacter()->SetPawnTeam(PlayerTeam);
	// 	}
	if (GetNetMode() == ENetMode::NM_Client)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Controller team info Replicated!"));
	}
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


void AINSPlayerController::ReceiveGameKills(class APlayerState* Killer, APlayerState* Victim, int32 Score, bool bIsTeamDamage)
{
	//get the local machine player controller
	AINSPlayerController* LocalPC = Cast<AINSPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	//if the killer is me
	if (Killer->PlayerId == LocalPC->PlayerState->PlayerId)
	{
		AINSHUDBase* PlayerHud = GetHUD<AINSHUDBase>();
		if (PlayerHud)
		{
			PlayerHud->SetStartDrawScore(true, Score);
			PlayerHud->SetStartDrawHitFeedBack(FLinearColor::Red);
		}
	}
}


void AINSPlayerController::ClientReceiveCauseDamage_Implementation(class AController* Victim, float DamageAmount, bool bIsTeamDamage)
{
	AINSHUDBase* PlayerHud = GetHUD<AINSHUDBase>();
	if (PlayerHud)
	{
		if (bIsTeamDamage)
		{
			PlayerHud->SetStartDrawHitFeedBack(FLinearColor::Blue);
		}
		else
		{
			PlayerHud->SetStartDrawHitFeedBack(DamageAmount >= 30.f ? FLinearColor::Yellow : FLinearColor::White);
		}
	}
}

bool AINSPlayerController::ClientReceiveCauseDamage_Validate(class AController* Victim, float DamageAmount, bool bIsTeamDamage)
{
	return true;
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
	// 	if (!NewWeaponToEquip)
	// 	{
	// 		NewWeaponToEquip = GetGameModeRandomWeapon();
	// 	}
	// 	if (NewWeaponToEquip)
	// 	{
	// 		GetINSPlayerCharacter()->SetCurrentWeapon(NewWeaponToEquip);
	// 	}
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

void AINSPlayerController::SetWeaponState(EWeaponState NewState)
{
	if (GetINSPlayerCharacter())
	{
		if (GetINSPlayerCharacter()->GetCurrentWeapon())
		{
			GetINSPlayerCharacter()->GetCurrentWeapon()->SetWeaponState(NewState);
		}
	}
}

void AINSPlayerController::ServerSetWeaponState_Implementation(EWeaponState NewState)
{
	SetWeaponState(NewState);
}

bool AINSPlayerController::ServerSetWeaponState_Validate(EWeaponState NewState)
{
	return true;
}

void AINSPlayerController::ReceiveEnterPickups(class AINSItems_Pickup* PickupItem)
{
	AINSHUDBase* PlayerHud = GetHUD<AINSHUDBase>();
	if (PlayerHud)
	{
		PlayerHud->SetPickupItemInfo(PickupItem->GetItemDisplayIcon(), true);
	}
}

void AINSPlayerController::ReceiveLeavePickups(class AINSItems_Pickup* PickupItem)
{
	AINSHUDBase* PlayerHud = GetHUD<AINSHUDBase>();
	if (PlayerHud)
	{
		PlayerHud->SetPickupItemInfo(nullptr, false);
	}
}

void AINSPlayerController::OnWeaponAmmoLeftEmpty()
{

}

void AINSPlayerController::OnWeaponClipAmmoLow()
{

}

void AINSPlayerController::OnWeaponClipEmpty(class AController* WeaponOwnerPlayer)
{
	if(WeaponOwnerPlayer&&WeaponOwnerPlayer==this)
	if (bAutoReload)
	{
		ReloadWeapon();
	}
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

void AINSPlayerController::GetGameModeRandomWeapon()
{
	const AINSGameModeBase* CurrentGameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
	if (CurrentGameMode)
	{
		PlayerDefaultWeaponClass = CurrentGameMode->GetRandomGameModeWeaponClass();
		AINSWeaponBase* const DefaultWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(PlayerDefaultWeaponClass, GetINSPlayerCharacter()->GetActorTransform(), this, GetINSPlayerCharacter(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (DefaultWeapon)
		{
			DefaultWeapon->SetWeaponState(EWeaponState::NONE);
			UGameplayStatics::FinishSpawningActor(DefaultWeapon, GetINSPlayerCharacter()->GetActorTransform());
			DefaultWeapon->SetOwner(this);
			DefaultWeapon->SetAutonomousProxy(true);
		}
		GetINSPlayerCharacter()->SetCurrentWeapon(DefaultWeapon);
	}
}

void AINSPlayerController::ServerGetGameModeRandomWeapon_Implementation()
{
	GetGameModeRandomWeapon();
}

bool AINSPlayerController::ServerGetGameModeRandomWeapon_Validate()
{
	return true;
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
		//UE_LOG(LogAINSPlayerController, Warning, TEXT("Player %s has see something ,other actor class:%s"), *this->GetName(), *Other->GetName())
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
	
}

AINSPlayerStateBase* AINSPlayerController::GetINSPlayerState()
{
	//this return value could still be nullptr 
	return INSPlayerState == nullptr ? Cast<AINSPlayerStateBase>(PlayerState) : INSPlayerState;
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

void AINSPlayerController::OnPlayerMeshSetupFinished()
{

}

void AINSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (InPawn)
	{
		AINSPlayerCharacter* const ControlledPawn = GetINSPlayerCharacter();
		if (ControlledPawn)
		{
			if (GetLocalRole() == ROLE_Authority)
			{
				ControlledPawn->SetCharacterTeam(PlayerTeam);
			}
		}
	}
}
