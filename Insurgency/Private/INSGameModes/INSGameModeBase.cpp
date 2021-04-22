// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameModes/INSGameModeBase.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSCharacter/INSCharacter.h"
#include "INSGameModes\INSGameStateBase.h"
#include "INSGameplay\INSTeamInfo.h"
#include "INSHud\INSHUDBase.h"
#include "Engine\World.h"
#include "INSCharacter\INSPlayerStateBase.h"
#include "INSDamageTypes\INSDamageType_Falling.h"
#include "INSPlayerSpawning/INSPlayerStart.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "..\..\Public\INSGameModes\INSGameModeBase.h"

DEFINE_LOG_CATEGORY(LogINSGameMode);
AINSGameModeBase::AINSGameModeBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
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
		SpawnTerrorisTeam();
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

void AINSGameModeBase::SpawnTerrorisTeam()
{
	if (TeamInfoClass)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to TeamInfo into a map	
		TerroristTeam = GetWorld()->SpawnActor<AINSTeamInfo>(TeamInfoClass);
		TerroristTeam->SetTeamType(ETeamType::REBEL);
		static const FString TeamKey = TEXT("Rebel");
		InGameTeams.Add(TeamKey, TerroristTeam);
		UE_LOG(LogINSGameMode, Log, TEXT("Allie Team Has spawned for this game"));
	}
}

void AINSGameModeBase::SpawnCounterTerroristTeam()
{
	if (TeamInfoClass)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save TeamInfo into a map	
		CTTeam = GetWorld()->SpawnActor<AINSTeamInfo>(TeamInfoClass);
		CTTeam->SetTeamType(ETeamType::ALLIE);
		static const FString TeamKey = TEXT("Allie");
		InGameTeams.Add(TeamKey, CTTeam);
		UE_LOG(LogINSGameMode, Log, TEXT("Rebel Team Has spawned for this game"));
	}
}

void AINSGameModeBase::PlayerScore(AINSPlayerController* ScoringPlayer)
{

}

void AINSGameModeBase::EndMatchPerparing()
{

}

void AINSGameModeBase::ModifyDamage(float& OutDamage, const float& OriginDamage, class AController* PlayerInstigator, class AController* Victim, const FDamageEvent& DamageEvent, const FName BoneName)
{
	//modify match state damage first
	if (GetMatchState() != MatchState::InProgress)
	{
		//do not apply any damage when game is not in progress
		OutDamage = 0.f;
	}
	bool bIsHeadShot = false;
	bool isDamageCausedByWorld = DamageEvent.DamageTypeClass->IsChildOf(UINSDamageType_Falling::StaticClass());
	//modify bone damage
	AINSCharacter* const VictimCharacter = Victim->GetPawn() == nullptr ? nullptr : CastChecked<AINSCharacter>(Victim->GetPawn());
	if (VictimCharacter)
	{
		FBoneDamageModifier BoneDamageModifierStruct;
		VictimCharacter->GetBoneDamageModifierStruct(BoneDamageModifierStruct);
		const float BoneDamageModifier = BoneDamageModifierStruct.GetBoneDamageModifier(BoneName);
		const float ModifiedDamage = OriginDamage * BoneDamageModifier;
		OutDamage = ModifiedDamage;
		bIsHeadShot = BoneName.ToString().Contains("Head", ESearchCase::IgnoreCase);
		UE_LOG(LogINSCharacter
			, Warning
			, TEXT("character %s hit with bone:%s,damage modifier values is:%f,Modified damage value is %f")
			, *GetName()
			, *BoneName.ToString()
			, BoneDamageModifier
			, ModifiedDamage);
	}
	//modify team damage
	const bool bIsTeamDamage = GetIsTeamDamage(PlayerInstigator, Victim) && !isDamageCausedByWorld;
	if (bIsTeamDamage)
	{
		if (bAllowTeamDamage)
		{
			OutDamage *= TeamDamageModifier;
			UE_LOG(LogINSGameMode, Log, TEXT("Modify Team damage from %f to %f for Victim Player %s"), OriginDamage, OutDamage, *Victim->GetName());
		}
		else
		{
			OutDamage = 0.f;
		}
	}
}

void AINSGameModeBase::ConfirmKill(AController* Killer, AController* Victim, int32 KillerScore, bool bIsTeamDamage)
{
	AINSGameStateBase* GS = GetGameState<AINSGameStateBase>();
	if (Killer->GetClass()->IsChildOf(AINSPlayerController::StaticClass()))
	{
		AINSPlayerStateBase* const KillerPlayerState = Cast<AINSPlayerController>(Killer)->GetINSPlayerState();
		if (KillerPlayerState)
		{
			const float PlayerCurrentScore = KillerPlayerState->GetScore();
			AINSTeamInfo* KillerTeam = Cast<AINSPlayerController>(Killer)->GetPlayerTeam();
			if (bIsTeamDamage)
			{
				KillerPlayerState->SetScore(PlayerCurrentScore + FMath::CeilToFloat(KillerScore));
				KillerPlayerState->AddKill(1);
				if (KillerTeam)
				{
					KillerTeam->AddTeamScore(FMath::CeilToFloat(KillerScore));
				}
			}
			else
			{
				KillerPlayerState->SetScore(FMath::Clamp<float>(PlayerCurrentScore - FMath::CeilToFloat(KillerScore), 0.f, PlayerCurrentScore));
				KillerTeam->AddTeamScore(FMath::Clamp<float>(KillerTeam->GetTeamScore() - FMath::CeilToFloat(KillerScore), 0.f, KillerTeam->GetTeamScore()));
			}
		}
	}
	if (Victim->GetClass()->IsChildOf(AINSPlayerController::StaticClass()))
	{
		AINSPlayerStateBase* const VictimPlayerState = Cast<AINSPlayerController>(Killer)->GetINSPlayerState();
		if (VictimPlayerState)
		{
			VictimPlayerState->AddDeath();
		}
	}

	if (GS)
	{
		GS->OnPlayerKilled(Killer, Victim, KillerScore, bIsTeamDamage);
	}
}

void AINSGameModeBase::GetInGameTeams(TMap<FString, AINSTeamInfo*>& OutTeams)
{
	OutTeams = InGameTeams;
}

bool AINSGameModeBase::GetIsTeamDamage(class AController* DamageInstigator, class AController* Victim)
{
	const class AINSPlayerController* const InstigatorPlayer = Cast<AINSPlayerController>(DamageInstigator);
	const class AINSPlayerController* const VictimPlayer = Cast<AINSPlayerController>(Victim);
	if (InstigatorPlayer && VictimPlayer)
	{
		const ETeamType InstigatorTeamType = InstigatorPlayer->GetPlayerState<AINSPlayerStateBase>()->GetPlayerTeam()->GetTeamType();
		const ETeamType VictimTeamType = VictimPlayer->GetPlayerState<AINSPlayerStateBase>()->GetPlayerTeam()->GetTeamType();
		UE_LOG(LogINSGameMode, Log, TEXT("player %s is causing Team damage to player %s"), *DamageInstigator->GetName(), *VictimPlayer->GetName());
		return InstigatorTeamType == VictimTeamType;
	}
	return false;
}

UClass* AINSGameModeBase::GetRandomGameModeWeaponClass() const
{
	const uint8 AvailableWeaponNum = GameModeAvailableWeaponsClasses.Num();
	if (AvailableWeaponNum > 0)
	{
		uint8 Ramdon = FMath::RandHelper(AvailableWeaponNum);
		return GameModeAvailableWeaponsClasses[Ramdon];
	}
	return nullptr;
}

void AINSGameModeBase::AssignPlayerTeam(class AINSPlayerController* NewPlayer)
{
	TArray<AINSPlayerStateBase*> CTTeamPlayers = CTTeam->TeamMembers;
	TArray<AINSPlayerStateBase*> TTeamPlayers = TerroristTeam->TeamMembers;
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

void AINSGameModeBase::ScorePlayer(class AINSPlayerController* PlayerToScore, int32 Score)
{

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
	AINSPlayerController* const PlayeController = Cast<AINSPlayerController>(Player);
	if (PlayeController)
	{
		AINSTeamInfo* PlayerTeam = PlayeController->GetPlayerTeam();
		TArray<APlayerStart*> RebelPlayerStarts;
		TArray<APlayerStart*> AlliePlayerStarts;
		//Iterate the all player start spots that placed in map in advance,
		//Match their tags,And categorize them
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* const Start = *It;
			if (Start)
			{
				if (Start->PlayerStartTag == FName(TEXT("Allie")))
				{
					AlliePlayerStarts.AddUnique(Start);
				}
				else if (Start->PlayerStartTag == FName(TEXT("Rebel")))
				{
					RebelPlayerStarts.AddUnique(Start);
				}
			}
		}
		const FName SelectePlayerTag = PlayerTeam->GetTeamType() == ETeamType::REBEL
			? TeamName::Rebel
			: TeamName::Allie;
		const TArray<APlayerStart*> SeletedStarts = PlayerTeam->GetTeamType() == ETeamType::REBEL
			? RebelPlayerStarts
			: AlliePlayerStarts;
		TArray<APlayerStart*> SafePlayerStarts;
		const TArray<TEnumAsByte <EObjectTypeQuery>> ObjectTypeQueries;
		TArray<AActor*> ActorsToIgnore;
		TArray<AActor*> FoundPlayers;
		if (SeletedStarts.Num() > 0)
		{
			for (int i = 0; i < SeletedStarts.Num(); i++)
			{
				APlayerStart* CurrentPlayerStart = SeletedStarts[i];
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
	CurrentGameState->SetMatchPrepareRemainingTime((uint8)FMath::CeilToInt(MatchPrepareRemainingTime));
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
