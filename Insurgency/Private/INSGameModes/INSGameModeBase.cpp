// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameModes/INSGameModeBase.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSCharacter/INSCharacter.h"
#include "INSGameModes\INSGameStateBase.h"
#include "INSGameplay\INSTeamInfo.h"
#include "INSHud\INSHUDBase.h"
#include "INSDamageTypes\INSDamageType_Falling.h"
#include "Engine\World.h"
#include "INSCharacter\INSPlayerStateBase.h"
#include "INSDamageTypes\INSDamageType_Falling.h"
#include "INSPlayerSpawning/INSPlayerStart.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "Insurgency/Insurgency.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"
#include "..\..\Public\INSGameModes\INSGameModeBase.h"

DEFINE_LOG_CATEGORY(LogINSGameMode);

AINSGameModeBase::AINSGameModeBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
	PlayerStateClass = AINSPlayerStateBase::StaticClass();
	PlayerControllerClass = AINSPlayerController::StaticClass();
	GameStateClass = AINSGameStateBase::StaticClass();
	DefaultPawnClass = AINSCharacter::StaticClass();
	HUDClass = AINSHUDBase::StaticClass();
	TeamInfoClass = AINSTeamInfo::StaticClass();
	bShouldDropWeaponWhenPlayerDead = true;
	bAllowTeamDamage = true;
	bAllowFallingDamage = true;
	TeamDamageModifier = 0.2f;
	PlayerMaxLives = 5;
	bAllowInfinitRespawn = true;
	MaximumPlayerAllowed = 32;
	bNeedCreateBot = false;
	CurrentBotNum = 0;
	KilledBotNum = 0;
	bAllowMove = true;
	bDefaultWeaponRequired = true;
	bIsLobbyGameMode = false;
	CurrentGameType = EGameType::PVP;
	bAllowFire = true;
	DefaultRestartTime = 5.f;
	bIsMatchPrepare = false;
	bMatchPreparingFinished = false;
	MatchPrepareTime = 5.f;
	MatchPrepareRemainingTime = MatchPrepareTime;
	bEnablePlayerSpawnDamageImmune = true;
}

void AINSGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	if (!bIsLobbyGameMode)
	{
		SpawnCounterTerroristTeam();
		SpawnTerroristTeam();
	}
	UE_LOG(LogINSGameMode, Log, TEXT("InitGame Finished"));
}

void AINSGameModeBase::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogINSGameMode, Log, TEXT("Start Play Game Finished"));
}

bool AINSGameModeBase::HasMatchStarted() const
{
	return Super::HasMatchStarted();
}

void AINSGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogINSGameMode, Log, TEXT("Player %s Has Called postLogin"), *NewPlayer->GetName());
}

void AINSGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void AINSGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	UE_LOG(LogINSGameMode, Log, TEXT("Player %s Has Called PreLogin"), *Address);
}

void AINSGameModeBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	InitDamageModifiers();
	bAllowFire = false;
	bAllowMove = false;
	AINSGameStateBase* CurrentGameState = GetGameState<AINSGameStateBase>();
	if (GameState)
	{
		CurrentGameState->SetRespawnTime(DefaultRestartTime);
		CurrentGameState->SetCTTeam(CTTeam);
		CurrentGameState->SetTerroristTeam(TerroristTeam);
		CurrentGameState->SetShouldDropWeaponWhenPlayerDead(bShouldDropWeaponWhenPlayerDead);
		CurrentGameState->SetGameDefaultRespawnTime(DefaultRestartTime);
		CurrentGameState->SetAllowFire(bAllowFire);
		CurrentGameState->SetAllowMove(bAllowMove);
		CurrentGameState->SetMatchPrepareRemainingTime(MatchPrepareTime);
	}
	UE_LOG(LogINSGameMode, Log, TEXT("PreInitializeComponents called"));
}

void AINSGameModeBase::SpawnTerroristTeam()
{
	if (TeamInfoClass)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient; // We never want save TeamInfo into a map	
		TerroristTeam = GetWorld()->SpawnActor<AINSTeamInfo>(TeamInfoClass);
		TerroristTeam->SetTeamType(ETeamType::REBEL);
		static const FString TeamKey = TEXT("Rebel");
		InGameTeams.Add(TeamKey, TerroristTeam);
		UE_LOG(LogINSGameMode, Log, TEXT("Allie Team Has spawned for this game"));
	}
}

void AINSGameModeBase::InitDamageModifiers()
{
	const uint8 NumDamageModifier = DamageModifiers.Num();
	if (NumDamageModifier > 0)
	{
		for (uint8 i = 0; i < NumDamageModifier; i++)
		{
			DamageModifierInstances.Add(NewObject<UINSDamageModifierBase>(this, DamageModifiers[i]));
			UE_LOG(LogINSGameMode, Log, TEXT("create game damge modifiers:%s"), *(DamageModifierInstances[i]->GetName()));
		}
	}
}

void AINSGameModeBase::SpawnCounterTerroristTeam()
{
	if (TeamInfoClass)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient; // We never want to save TeamInfo into a map	
		CTTeam = GetWorld()->SpawnActor<AINSTeamInfo>(TeamInfoClass);
		CTTeam->SetTeamType(ETeamType::ALLIE);
		static const FString TeamKey = TEXT("Allie");
		InGameTeams.Add(TeamKey, CTTeam);
		UE_LOG(LogINSGameMode, Log, TEXT("Rebel Team Has spawned for this game"));
	}
}


void AINSGameModeBase::EndMatchPreparing()
{
}

float AINSGameModeBase::ModifyDamage(float InDamage, class AController* PlayerInstigator, class AController* Victim, const struct FDamageEvent& DamageEvent)
{
	if (!Victim)
	{
		return 0.f;
	}

	//modify match state damage first
	if (GetMatchState() != MatchState::InProgress)
	{
		//do not apply any damage when game is not in progress
		return 0.f;
	}
	const AINSCharacter* const Character = CastChecked<AINSCharacter>(Victim->GetPawn());
	if (!Character || Character->GetIsDead())
	{
		return 0.f;
	}

	float ActualDamageToApply = InDamage;
	const uint8 NumDamageModifier = DamageModifierInstances.Num();
	if (NumDamageModifier > 0)
	{
		for (uint8 i = 0; i < NumDamageModifier; i++)
		{
			if (DamageModifierInstances[i])
			{
				DamageModifierInstances[i]->ModifyDamage(ActualDamageToApply, (FDamageEvent&)DamageEvent, PlayerInstigator, Victim);
			}
		}
	}
	//handle falling damage
	const UClass* const DmgTypeClass = DamageEvent.DamageTypeClass;
	const bool bDamageCausedByWorld = DmgTypeClass && DmgTypeClass->IsChildOf(UINSDamageType_Falling::StaticClass());
	bool bIsHeadShot = false;
	//modify bone damage
	AINSCharacter* const VictimCharacter = Victim->GetPawn() == nullptr ? nullptr : CastChecked<AINSCharacter>(Victim->GetPawn());
	const bool bIsTeamDamage = GetIsTeamDamage(PlayerInstigator, Victim) && !bDamageCausedByWorld;
	if (bIsTeamDamage)
	{
		if (bAllowTeamDamage)
		{
			ActualDamageToApply *= TeamDamageModifier;
			UE_LOG(LogINSGameMode, Log, TEXT("Modify Team damage from %f to %f for Victim Player %s"), ActualDamageToApply, *Victim->GetName());
		}
		else
		{
			ActualDamageToApply = 0.f;
		}
	}
	return ActualDamageToApply;
}

void AINSGameModeBase::PlayerScore(class AController* ScorePlayer, class AController* Victim, const struct FTakeHitInfo& HitInfo)
{
	AINSGameStateBase* GS = GetGameState<AINSGameStateBase>();
	if (ScorePlayer->GetClass()->IsChildOf(AINSPlayerController::StaticClass()))
	{
		AINSPlayerStateBase* const KillerPlayerState = Cast<AINSPlayerController>(ScorePlayer)->GetINSPlayerState();
		if (KillerPlayerState)
		{
			const float PlayerCurrentScore = KillerPlayerState->GetScore();
			AINSTeamInfo* KillerTeam = Cast<AINSPlayerController>(ScorePlayer)->GetPlayerTeam();
			if (HitInfo.bIsTeamDamage)
			{
				KillerPlayerState->SetScore(PlayerCurrentScore + FMath::CeilToFloat(-100.f));
				KillerPlayerState->AddKill(1);
				if (KillerTeam)
				{
					KillerTeam->AddTeamScore(FMath::CeilToFloat(-100.f));
				}
			}
			else
			{
				KillerPlayerState->SetScore(FMath::Clamp<float>(PlayerCurrentScore - FMath::CeilToFloat(100.f), 0.f, PlayerCurrentScore));
				KillerTeam->AddTeamScore(FMath::Clamp<float>(KillerTeam->GetTeamScore() - FMath::CeilToFloat(100.f), 0.f, KillerTeam->GetTeamScore()));
			}
		}
	}
}

void AINSGameModeBase::ConfirmPlayerKill(class AController* ScorePlayer, class AController* Victim, const FTakeHitInfo& HitInfo)
{
	// separate player and victim handling because there player controller class may not the same
	// for example,one player controller and one ai controller
	if (ScorePlayer->GetClass()->IsChildOf(AINSPlayerController::StaticClass()))
	{
		AINSPlayerStateBase* const KillerPlayerState = Cast<AINSPlayerController>(ScorePlayer)->GetINSPlayerState();
		if (KillerPlayerState)
		{
			HitInfo.bIsTeamDamage ? KillerPlayerState->AddMissTakeKill() : KillerPlayerState->AddKill();
		}
	}

	if (Victim->GetClass()->IsChildOf(AINSPlayerController::StaticClass()))
	{
		AINSPlayerStateBase* const VictimPlayerState = Cast<AINSPlayerController>(Victim)->GetINSPlayerState();
		if (VictimPlayerState)
		{
			VictimPlayerState->AddDeath();
		}
	}
}

void AINSGameModeBase::GetInGameTeams(TMap<FString, AINSTeamInfo*>& OutTeams)
{
	OutTeams = InGameTeams;
}

bool AINSGameModeBase::GetIsTeamDamage(class AController* DamageInstigator, class AController* Victim)
{
	if (DamageInstigator && Victim)
	{
		const UClass* InstigatorClass = DamageInstigator->GetClass();
		const UClass* VictimClass = Victim->GetClass();
		if (InstigatorClass->IsChildOf(AINSPlayerController::StaticClass()) && VictimClass->IsChildOf(AINSPlayerController::StaticClass()))
		{
			const AINSPlayerStateBase* const InstigatorPlayerState = DamageInstigator->GetPlayerState<AINSPlayerStateBase>();
			const AINSPlayerStateBase* const VictimPlayerState = Victim->GetPlayerState<AINSPlayerStateBase>();
			const AINSTeamInfo* const InstigatorTeam = InstigatorPlayerState == nullptr ? nullptr : InstigatorPlayerState->GetPlayerTeam();
			const AINSTeamInfo* const VictimTeam = VictimPlayerState == nullptr ? nullptr : VictimPlayerState->GetPlayerTeam();
			if (InstigatorPlayerState && VictimPlayerState && InstigatorTeam && VictimTeam && InstigatorTeam->GetTeamType() == VictimTeam->GetTeamType())
			{
				return true;
			}
		}
	}
	return false;
}

UClass* AINSGameModeBase::GetRandomGameModeWeaponClass() const
{
	const uint8 AvailableWeaponNum = GameModeAvailableWeaponsClasses.Num();
	if (AvailableWeaponNum > 0)
	{
		const uint8 Random = static_cast<uint8>(FMath::RandHelper(AvailableWeaponNum));
		return GameModeAvailableWeaponsClasses[Random];
	}
	return nullptr;
}

UDataTable* AINSGameModeBase::GetWeaponDataTable() const
{
	return WeaponDataTable;
}

void AINSGameModeBase::AssignPlayerTeam(class AINSPlayerController* NewPlayer)
{
	const TArray<AINSPlayerStateBase*> CTTeamPlayers = CTTeam->TeamMembers;
	const TArray<AINSPlayerStateBase*> TTeamPlayers = TerroristTeam->TeamMembers;
	const uint8 CTTeamPlayerNum = CTTeamPlayers.Num();
	const uint8 TTeamPlayerNum = TTeamPlayers.Num();
	static const FString RebelsTeamKey = TeamName::Rebel.ToString();
	static const FString AlliesTeamKey = TeamName::Allie.ToString();
	//balance Player team
	AINSTeamInfo* SelectedTeam = nullptr;
	if (CTTeamPlayerNum == TTeamPlayerNum)
	{
		const bool RandomBool = FMath::RandBool();
		SelectedTeam = RandomBool ? GetGameState<AINSGameStateBase>()->GetCTTeamInfo() : GetGameState<AINSGameStateBase>()->GetTerroristTeamInfo();
	}
	else if (CTTeamPlayerNum > TTeamPlayerNum)
	{
		SelectedTeam = GetGameState<AINSGameStateBase>()->GetTerroristTeamInfo();
	}
	else if (CTTeamPlayerNum < TTeamPlayerNum)
	{
		SelectedTeam = GetGameState<AINSGameStateBase>()->GetCTTeamInfo();
	}
	if (SelectedTeam && NewPlayer)
	{
		NewPlayer->SetPlayerTeam(SelectedTeam);
	}
}

void AINSGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bIsMatchPrepare && IsMatchInProgress() && !bMatchPreparingFinished)
	{
		bIsMatchPrepare = true;
		AINSGameStateBase* const CurrentGameState = GetGameState<AINSGameStateBase>();
		if (CurrentGameState)
		{
			CurrentGameState->SetPreparingMatch(true);
		}
		if (!GetWorldTimerManager().IsTimerActive(MatchPrepareTimer))
		{
			GetWorldTimerManager().SetTimer(MatchPrepareTimer, this, &AINSGameModeBase::CountDownMatchPrepare, 1.f, true, 0.f);
		}
	}
}


APlayerController* AINSGameModeBase::SpawnPlayerControllerCommon(ENetRole InRemoteRole, FVector const& SpawnLocation, FRotator const& SpawnRotation, TSubclassOf<APlayerController> InPlayerControllerClass)
{
	APlayerController* SpawnedPlayerController = Super::SpawnPlayerControllerCommon(InRemoteRole, SpawnLocation, SpawnRotation, InPlayerControllerClass);
	if (InPlayerControllerClass->IsChildOf(AINSPlayerController::StaticClass()))
	{
		AINSPlayerController* PlayerController = Cast<AINSPlayerController>(SpawnedPlayerController);
		AssignPlayerTeam(PlayerController);
	}
	return SpawnedPlayerController;
}

AActor* AINSGameModeBase::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName /* = TEXT("") */)
{
	if (Player == nullptr)
	{
		UE_LOG(LogINSGameMode, Warning, TEXT("Trying to find a player start spot for null player,abort custom logic!"));
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}

	UWorld* const World = GetWorld();
	AINSPlayerController* const PlayerController = Cast<AINSPlayerController>(Player);
	if (PlayerController)
	{
		AINSTeamInfo* PlayerTeam = PlayerController->GetPlayerTeam();
		TArray<APlayerStart*> RebelPlayerStarts;
		TArray<APlayerStart*> AlliePlayerStarts;
		//Iterate the all player start spots that placed in map in advance,
		//Match their tags,And categorize them
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* const PlayerStart = *It;
			if (PlayerStart)
			{
				if (PlayerStart->PlayerStartTag == FName(TEXT("Allie")))
				{
					AlliePlayerStarts.AddUnique(PlayerStart);
				}
				else if (PlayerStart->PlayerStartTag == FName(TEXT("Rebel")))
				{
					RebelPlayerStarts.AddUnique(PlayerStart);
				}
			}
		}
		const FName SelectedPlayerTag = PlayerTeam->GetTeamType() == ETeamType::REBEL ? TeamName::Rebel : TeamName::Allie;
		const TArray<APlayerStart*> SelectedStarts = PlayerTeam->GetTeamType() == ETeamType::REBEL ? RebelPlayerStarts : AlliePlayerStarts;
		TArray<APlayerStart*> SafePlayerStarts;
		const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeQueries;
		TArray<AActor*> ActorsToIgnore;
		TArray<AActor*> FoundPlayers;
		if (SelectedStarts.Num() > 0)
		{
			for (int i = 0; i < SelectedStarts.Num(); i++)
			{
				APlayerStart* CurrentPlayerStart = SelectedStarts[i];
				UKismetSystemLibrary::SphereOverlapActors(GetWorld()
				                                          , CurrentPlayerStart->GetActorLocation()
				                                          , 2000.f
				                                          , ObjectTypeQueries
				                                          , AINSPlayerController::StaticClass()
				                                          , ActorsToIgnore
				                                          , FoundPlayers);
				if (FoundPlayers.Num() == 0)
				{
					SafePlayerStarts.AddUnique(CurrentPlayerStart);
				}
				else if (FoundPlayers.Num() > 0)
				{
					bool bContainsEnemy = false;
					for (int j = 0; j < FoundPlayers.Num(); j++)
					{
						AINSPlayerController* const ThatPlayer = Cast<AINSPlayerController>(FoundPlayers[i]);
						const AINSTeamInfo* const ThatPlayerTeam = ThatPlayer->GetPlayerTeam();
						if (PlayerTeam->GetTeamType() != ThatPlayerTeam->GetTeamType())
						{
							bContainsEnemy = true;
						}
					}
					if (!bContainsEnemy)
					{
						SafePlayerStarts.AddUnique(CurrentPlayerStart);
					}
				}
			}
			if (SafePlayerStarts.Num() > 0)
			{
				const int32 Random = FMath::RandHelper(SafePlayerStarts.Num() - 1);
				return SafePlayerStarts[Random];
			}
			else
			{
				return Super::FindPlayerStart_Implementation(Player, IncomingName);
			}
		}
	}
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void AINSGameModeBase::CountDownMatchPrepare()
{
	MatchPrepareRemainingTime -= 1.f;
	AINSGameStateBase* const CurrentGameState = GetGameState<AINSGameStateBase>();
	//compress to uint8 
	CurrentGameState->SetMatchPrepareRemainingTime(static_cast<uint8>(FMath::CeilToInt(MatchPrepareRemainingTime)));
	if (MatchPrepareRemainingTime == 0)
	{
		GetWorldTimerManager().ClearTimer(MatchPrepareTimer);
		bIsMatchPrepare = false;
		bMatchPreparingFinished = true;
		bAllowMove = true;
		bAllowFire = true;
		CurrentGameState->SetAllowFire(bAllowFire);
		CurrentGameState->SetAllowMove(bAllowMove);
		CurrentGameState->SetPreparingMatch(bIsMatchPrepare);
	}
}
