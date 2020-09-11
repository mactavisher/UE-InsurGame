// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameModes/INSGameModeBase.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSCharacter/INSCharacter.h"
#include "INSGameModes\INSGameStateBase.h"
#include "INSGameplay\INSTeamInfo.h"
#include "INSHud\INSHUDBase.h"
#include "Engine\World.h"
#include "INSCharacter\INSPlayerStateBase.h"
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
	static const FString CTTeamKey = TEXT("CT");
	static const FString TTeamKey = TEXT("T");
	const bool RandomBool = FMath::RandBool();
	class AINSTeamInfo* SelectedTeam = RandomBool ? GetGameState<AINSGameStateBase>()->GetCTTeamInfo() : GetGameState<AINSGameStateBase>()->GetTerroristTeamInfo();
	class AINSPlayerController* Player = Cast<AINSPlayerController>(NewPlayer);
	if (Player)
	{
		AINSPlayerStateBase* PlayerState = CastChecked<AINSPlayerStateBase>(NewPlayer->PlayerState);
		SelectedTeam->AddPlayerToThisTeam(Player);
		Player->SetPlayerTeam(SelectedTeam);
	}
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
		//CurrentGameState->SetAllowFire(bAllowFire);
		//CurrentGameState->SetAllowMove(bAllowMove);
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
		SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save game states or network managers into a map	
		TerroristTeam = GetWorld()->SpawnActor<AINSTeamInfo>(TeamInfoClass);
		TerroristTeam->SetTeamType(ETeamType::T);
		static const FString TeamKey = TEXT("T");
		InGameTeams.Add(TeamKey, TerroristTeam);
		UE_LOG(LogINSGameMode, Log, TEXT("Terrorist Team Has spawned for this game"));
	}
}

void AINSGameModeBase::SpawnCounterTerroristTeam()
{
	if (TeamInfoClass)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = GetInstigator();
		SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save game states or network managers into a map	
		CTTeam = GetWorld()->SpawnActor<AINSTeamInfo>(TeamInfoClass);
		CTTeam->SetTeamType(ETeamType::CT);
		static const FString TeamKey = TEXT("CT");
		InGameTeams.Add(TeamKey, CTTeam);
		UE_LOG(LogINSGameMode, Log, TEXT("Counter Terrorist Team Has spawned for this game"));
	}
}

void AINSGameModeBase::PlayerScore(AINSPlayerController* ScoringPlayer)
{

}

void AINSGameModeBase::EndMatchPerparing()
{

}

void AINSGameModeBase::ModifyDamage(float& OutDamage, AController* PlayerInstigator, AController* Victim, FName BoneName)
{
	//modify match state damage first
	if (GetMatchState() != MatchState::InProgress)
	{
		//do not apply any damage when game is not in progress
		OutDamage = 0.f;
	}
	float originDamage = OutDamage;
	bool bIsHeadShot = false;
	//modify bone damage
	AINSCharacter* const VictimCharacter = Victim->GetPawn() == nullptr ? nullptr : CastChecked<AINSCharacter>(Victim->GetPawn());
	if (VictimCharacter)
	{
		FBoneDamageModifier BoneDamageModifierStruct;
		VictimCharacter->GetBoneDamageModifierStruct(BoneDamageModifierStruct);
		const float BoneDamageModifier = BoneDamageModifierStruct.GetBoneDamageModifier(BoneName);
		const float ModifiedDamage = OutDamage * BoneDamageModifier;
		OutDamage = ModifiedDamage;
		if (BoneName.ToString().Contains("Head", ESearchCase::IgnoreCase))
		{
			bIsHeadShot = true;
		}
		UE_LOG(LogINSCharacter, Warning, TEXT("character %s hit with bone:%s,damage modifier values is:%f,Modified damage value is %f"), *GetName(), *BoneName.ToString(), BoneDamageModifier, ModifiedDamage);
	}
	//modify team damage
	const bool bIsTeamDamage = GetIsTeamDamage(PlayerInstigator, Victim);
	if (bIsTeamDamage)
	{
		if (bAllowTeamDamage)
		{
			OutDamage *= TeamDamageModifier;
			UE_LOG(LogINSGameMode, Log, TEXT("Modify Team damage from %f to %f for Victim Player %s"), originDamage, OutDamage, *Victim->GetName());
		}
		else
		{
			OutDamage = 0.f;
		}
	}
}

void AINSGameModeBase::ConfirmKill(AController* Killer, AController* Victim,int32 KillerScore, bool bIsTeamDamage)
{
	AINSGameStateBase* GS = GetGameState<AINSGameStateBase>();
	if (GS)
	{
		GS->OnPlayerKilled(Killer, Victim,KillerScore,bIsTeamDamage);
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
	if (InstigatorPlayer&&VictimPlayer)
	{
		const ETeamType InstigatorTeamType = InstigatorPlayer->GetPlayerState<AINSPlayerStateBase>()->GetPlayerTeam()->GetTeamType();
		const ETeamType VictimTeamType = VictimPlayer->GetPlayerState<AINSPlayerStateBase>()->GetPlayerTeam()->GetTeamType();
		return InstigatorTeamType == VictimTeamType;
		UE_LOG(LogINSGameMode, Log, TEXT("player %s is causing Team damage to player %s"), *DamageInstigator->GetName(), *VictimPlayer->GetName());
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

}

void AINSGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bIsMatchPrepare&&IsMatchInProgress() && !bMatchPreparingFinished)
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
