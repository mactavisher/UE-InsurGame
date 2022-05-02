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
#include "INSPickups/INSPickupBase.h"
#include "Components/AudioComponent.h"
#include "INSAI/AIZombie/INSZombie.h"
#ifndef GEngine
#include "Engine/Engine.h"
#endif
DEFINE_LOG_CATEGORY(LogAINSPlayerController);

AINSPlayerController::AINSPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AINSPlayerCameraManager::StaticClass();
	bPlayerFiring = false;
	AmbientAudioComp = ObjectInitializer.CreateDefaultSubobject<UAudioComponent>(this, TEXT("AmbientAudioComp"));
	bAutoReload = true;
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
	InputComponent->BindAction("Jump", IE_Pressed, this, &AINSPlayerController::Jump);
	InputComponent->BindAction("EquipSlotItem_1", IE_Pressed, this, &AINSPlayerController::EquipSlotItem_1);
	InputComponent->BindAction("EquipSlotItem_2", IE_Pressed, this, &AINSPlayerController::EquipSlotItem_2);
	InputComponent->BindAction("EquipSlotItem_3", IE_Pressed, this, &AINSPlayerController::EquipSlotItem_3);
	InputComponent->BindAction("EquipSlotItem_4", IE_Pressed, this, &AINSPlayerController::EquipSlotItem_4);
	InputComponent->BindAction("EquipSlotItem_5", IE_Pressed, this, &AINSPlayerController::EquipSlotItem_5);
	InputComponent->BindAction("EquipSlotItem_6", IE_Pressed, this, &AINSPlayerController::EquipSlotItem_6);
	UE_LOG(LogAINSPlayerController, Log, TEXT("finish Setting up and bind Player inputs"));
}

void AINSPlayerController::MoveRight(float Value)
{
	const AINSGameStateBase* GameState = GetWorld()->GetGameState<AINSGameStateBase>();
	if (GameState && GameState->GetAllowMove())
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
	if (GameState && GameState->GetAllowMove() && Value != 0.f)
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
		CurrentPlayerState->SetScore(CurrentPlayerState->GetScore() + Score);
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
		AINSPlayerStateBase* MyPlayerState = CastChecked<AINSPlayerStateBase>(GetPlayerState<AINSPlayerStateBase>());
		PlayerTeam->AddPlayerToThisTeam(GetINSPlayerState());
		MyPlayerState->SetPlayerTeam(PlayerTeam);
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

class AINSTeamInfo* AINSPlayerController::GetPlayerTeam()
{
	return PlayerTeam;
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
	if (GetINSPlayerCharacter())
	{
		GetINSPlayerCharacter()->HandleCrouchRequest();
	}
}

void AINSPlayerController::UnCrouch()
{
	if (GetINSPlayerCharacter())
	{
		GetINSPlayerCharacter()->HandleCrouchRequest();
	}
}


void AINSPlayerController::EquipSlotItem_1()
{
	EquipWeapon(1);
}

void AINSPlayerController::EquipSlotItem_2()
{
	EquipWeapon(2);
}

void AINSPlayerController::EquipSlotItem_3()
{
	EquipWeapon(3);
}

void AINSPlayerController::EquipSlotItem_4()
{
	EquipWeapon(4);
}

void AINSPlayerController::EquipSlotItem_5()
{
	EquipWeapon(5);
}

void AINSPlayerController::EquipSlotItem_6()
{
	EquipWeapon(6);
}

void AINSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSPlayerController, PlayerTeam);
}

void AINSPlayerController::AimWeapon()
{
	if (GetINSPlayerCharacter())
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			GetINSPlayerCharacter()->HandleAimWeaponRequest();
		}
		else if (GetLocalRole() == ROLE_AutonomousProxy)
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
	if (GetINSPlayerCharacter())
	{
		if (HasAuthority())
		{
			GetINSPlayerCharacter()->HandleWeaponReloadRequest();
		}
		else if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			ServerReloadWeapon();
		}
	}
}

void AINSPlayerController::Fire()
{
	const AINSGameStateBase* const CurrentGameState = GetWorld()->GetGameState<AINSGameStateBase>();
	if (GetINSPlayerCharacter() && CurrentGameState && CurrentGameState->GetAllowFire())
	{
		GetINSPlayerCharacter()->HandleFireRequest();
	}
}

void AINSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AINSPlayerController::OnRep_PlayerTeam()
{
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


void AINSPlayerController::StopFire()
{
	if (GetINSPlayerCharacter())
	{
		GetINSPlayerCharacter()->HandleStopFireRequest();
	}
}

void AINSPlayerController::Sprint()
{
	if (HasAuthority())
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

void AINSPlayerController::Jump()
{
	if (GetINSPlayerCharacter())
	{
		GetINSPlayerCharacter()->HandleJumpRequest();
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
	UE_LOG(LogINSCharacter, Log, TEXT("handle weapon switch fire mode request"));
	if (HasAuthority())
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


void AINSPlayerController::PlayerCauseDamage(const FTakeHitInfo& HitInfo)
{
	AINSHUDBase* const PlayerHud = GetHUD<AINSHUDBase>();
	if (PlayerHud)
	{
		if (HitInfo.bIsTeamDamage)
		{
			PlayerHud->SetStartDrawHitFeedBack(FLinearColor::Blue);
		}
		else
		{
			if (HitInfo.bVictimDead && !HitInfo.bVictimAlreadyDead)
			{
				//PlayerHud->SetStartDrawScore(true, Score);
				PlayerHud->SetStartDrawHitFeedBack(FLinearColor::Red);
			}
			if (!HitInfo.bVictimDead)
			{
				PlayerHud->SetStartDrawHitFeedBack(HitInfo.Damage >= 30.f ? FLinearColor::Yellow : FLinearColor::White);
			}
		}
	}
}


void AINSPlayerController::InspectWeapon()
{
}

void AINSPlayerController::ServerInspectWeapon_Implementation()
{
	if (HasAuthority())
	{
	}
	else
	{
		InspectWeapon();
	}
}

bool AINSPlayerController::ServerInspectWeapon_Validate()
{
	return true;
}

void AINSPlayerController::PickupWeapon(class AINSPickup_Weapon* NewWeaponPickup)
{
	/*if (GetLocalRole() == ROLE_Authority)
	{
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
	}*/
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


void AINSPlayerController::EquipWeapon(const uint8 SlotIndex)
{
	if (GetINSPlayerCharacter())
	{
		GetINSPlayerCharacter()->HandleItemEquipRequest(0, SlotIndex);
	}
}

void AINSPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AINSPlayerController::ServerEquipWeapon_Implementation(const uint8 SlotIndex)
{
	EquipWeapon(SlotIndex);
}

bool AINSPlayerController::ServerEquipWeapon_Validate(const uint8 SlotIndex)
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

void AINSPlayerController::ReceiveEnterPickups(class AINSPickupBase* PickupItem)
{
	AINSHUDBase* PlayerHud = GetHUD<AINSHUDBase>();
	if (PlayerHud)
	{
	}
}

void AINSPlayerController::ReceiveLeavePickups(class AINSPickupBase* PickupItem)
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
	if (CurrentWeapon->GetOwner() == this)
	{
		if (bAutoReload)
		{
			ReloadWeapon();
		}
	}
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

UClass* AINSPlayerController::GetGameModeRandomWeapon()
{
	if (HasAuthority())
	{
		const AINSGameModeBase* const CurrentGameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
		if (CurrentGameMode)
		{
			return PlayerDefaultWeaponClass = CurrentGameMode->GetRandomGameModeWeaponClass();
		}
	}
	return nullptr;
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
	if (Other->GetClass() == AINSZombie::StaticClass())
	{
		return true;
	}
	bool bIsOtherAEnemy = false;
	const UClass* const OtherActorClass = Other->GetClass();
	if (OtherActorClass->IsChildOf(AINSPlayerCharacter::StaticClass()))
	{
		const AINSPlayerCharacter* const OtherPlayerCharacter = CastChecked<AINSPlayerCharacter>(Other);
		const AINSPlayerStateBase* const OtherPlayerState = OtherPlayerCharacter->GetPlayerState() == nullptr ? nullptr : OtherPlayerCharacter->GetPlayerStateChecked<AINSPlayerStateBase>();
		const AINSPlayerStateBase* const MyPlayerState = GetINSPlayerCharacter()->GetPlayerState() == nullptr ? nullptr : GetINSPlayerCharacter()->GetPlayerStateChecked<AINSPlayerStateBase>();
		if (OtherPlayerState && MyPlayerState)
		{
			const AINSTeamInfo* const OtherPlayerCharacterTeam = OtherPlayerState->GetPlayerTeam();
			const AINSTeamInfo* const MyPlayerCharacterTeam = MyPlayerState->GetPlayerTeam();
			if (OtherPlayerCharacterTeam && MyPlayerCharacterTeam)
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

bool AINSPlayerController::HasSeeEnemy()
{
	return IsEnemyFor(GetViewTarget());
}


AINSPlayerStateBase* AINSPlayerController::GetINSPlayerState()
{
	//this return value could still be nullptr 
	return INSPlayerState == nullptr ? GetPlayerState<AINSPlayerStateBase>() : INSPlayerState;
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
	if (HasAuthority())
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
	PossessedINSCharacter = Cast<AINSPlayerCharacter>(InPawn);
	if (PossessedINSCharacter)
	{
		GetINSPlayerCharacter()->SetTeamType(PlayerTeam->GetTeamType());
	}
	Super::OnPossess(InPawn);
}

void AINSPlayerController::OnCharacterDead()
{
	if (HasAuthority())
	{
		GetINSPlayerState()->OnPawnCharDeath();
		UnPossess();
	}
}

void AINSPlayerController::ReceiveCurrentClipAmmoEmpty()
{
	ReloadWeapon();
}
