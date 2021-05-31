// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "INSGameModeBase.generated.h"


class AINSWeaponBase;
class AINSTeamInfo;
/**
 *
 */
INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSGameMode, Log, All);

UCLASS()
class INSURGENCY_API AINSGameModeBase : public AGameMode
{
	GENERATED_UCLASS_BODY()

public:

	/** indicate if this game allow Team Damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TeamDamage")
		uint8 bAllowTeamDamage : 1;

	/** indicates if this game enable a falling damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TeamDamage")
		uint8 bAllowFallingDamage : 1;

	/** indicates if this game enable a falling damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TeamDamage")
		uint8 bAllowInfinitRespawn : 1;

	/** indicates if this game enable a falling damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TeamDamage")
		uint8 bIsLobbyGameMode : 1;

	/** if team damage is enabled ,the damage will need to apply a modifier  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TeamDamage")
		float TeamDamageModifier;

	/** player maximum lives that can ReSpawn during a level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameModeConfig")
		uint8 PlayerMaxLives;

	/** player maximum lives that can ReSpawn during a level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameModeConfig")
		uint8 MaximumPlayerAllowed;

	/** indicate if player can re_spawn infinite times when ever they are dead */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameModeConfig")
		uint8 bInfinitRespawn : 1;

	/** AI bot class that will be used in this game */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bot")
		TSubclassOf<ACharacter> BotCharacterClass;

	/** team info class */
	UPROPERTY(EditAnywhere, NoClear, BlueprintReadOnly, Category = Classes)
		TSubclassOf<AINSTeamInfo> TeamInfoClass;

	/** indicate if this game need bot to play with */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Config)
		uint8 bNeedCreateBot : 1;

	/** the maximum bot number allowed in the game at a specific game stage or state  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bot")
		int32 MaxBotNum;

	/** current bot number in game  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bot")
		int32 CurrentBotNum;

	/** accumulated bot that been killed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bot")
		int32 KilledBotNum;

	/** dose this game mode requires a default weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameModeConfig")
		uint8 bDefaultWeaponRequired : 1;

	/** dose this game mode requires a default weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameModeConfig")
		uint8 bAllowFire : 1;

	/** dose this game mode requires a default weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameModeConfig")
		uint8 bAllowMove : 1;

	/** the maximum bot number allowed in the game at a specific game stage or state  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bot")
		uint8 MaxSingleTeamPlayers;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Team")
		TMap<FString, AINSTeamInfo*> InGameTeams;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameModeConfig")
		EGameType CurrentGameType;

	/** default respawn time for a dead player */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameModeConfig")
		float DefaultRestartTime;

	/** indicate whether player should drop their weapon as a pick up,will be replicated via game state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Config)
		uint8 bShouldDropWeaponWhenPlayerDead : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
		AINSTeamInfo* TerroristTeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
		AINSTeamInfo* CTTeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameState")
		uint8 bIsMatchPrepare : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameState")
		uint8 bMatchPreparingFinished : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PrepareTime")
		float MatchPrepareTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PreParingTime")
		float MatchPrepareRemainingTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DamageControl")
		uint8 bEnablePlayerSpawnDamageImmune : 1;

	UPROPERTY()
		FTimerHandle MatchPrepareTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestWeaponClasses")
		TArray<TSubclassOf< AINSWeaponBase>> GameModeAvailableWeaponsClasses;

protected:

	//~ Begin AActor interface
	virtual void Tick(float DeltaSeconds)override;
	virtual void PreInitializeComponents()override;
	//~ end AActor interface

	//~ begin AGameMode interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual bool HasMatchStarted() const override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)override;
	virtual void EndMatchPerparing();
	/**
	 * @desc  Override spawns player controller and assign the player team and other attribute if need
	 * @param InRemoteRole RemoteRole
	 * @param SpawnLocation Location to Spawn this Controller
	 * @param SpawnRotation Rotation to spawn this controller
	 * @param InPlayerControllerClass  Player Controller type to spawn
	 */
	virtual APlayerController* SpawnPlayerControllerCommon(ENetRole InRemoteRole, FVector const& SpawnLocation, FRotator const& SpawnRotation, TSubclassOf<APlayerController> InPlayerControllerClass)override;

	/**
	 * @desc override and find a suitable player start to spawn the player controller
	 * @param Player
	 * @param IncomingName
	 */
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName /* = TEXT("") */)override;
	//~ end AGameMode interface

	/**
	 * @Desc add the specific player to the team
	 * @Param NewPlayer The player to add to team
	 */
	virtual void AssignPlayerTeam(class AINSPlayerController* NewPlayer);

	/** creates a terrorist Team for this game */
	virtual void SpawnTerrorisTeam();

	/** creates a counterTerroristTeam for this game  */
	virtual void SpawnCounterTerroristTeam();

	UFUNCTION()
		virtual void CountDownMatchPrepare();

public:

	/**
	 * @Desc  Modify damage according to the game rules
	 * @Param OutDamage out produced damage after damage is modified
	 * @Param OriginDamage damage before modified
	 * @Param PlayerInstigator Player Who actually instigate the damage
	 * @Param Victim The victim player
	 * @Param Damage event associate with this damage
	 * @Param BoneName The bone that hit with this damage
	 */
	virtual void ModifyDamage(float& OutDamage, const float& OriginDamage, class AController* PlayerInstigator, class AController* Victim, const FDamageEvent& DamageEvent, const FName BoneName);

	/**
	 * @Desc  confirms a player kill
	 * @Param OutDamage out produced damage after damage is modified
	 * @Param Killer Player Who make that kill
	 * @Param Victim The victim player
	 * @Param KillerScore  score
	 * @Param bIsTeamDamage is this kill is team mate kill
	 */
	virtual void PlayerScore(class AController* ScorePlayer, class AController* Victim, const FTakeHitInfo& HitInfo);


	/**
	 * @Desc  confirms a player kill
	 * @Param OutDamage out produced damage after damage is modified
	 * @Param Killer Player Who make that kill
	 * @Param Victim The victim player
	 * @Param KillerScore  score
	 * @Param bIsTeamDamage is this kill is team mate kill
	 */
	virtual void ConfirmPlayerKill(class AController* ScorePlayer, class AController* Victim, const struct FTakeHitInfo& HitInfo);

	/**
	 * @Desc  get teams for this game
	 * @Param OutTeams out team
	 */
	virtual void GetInGameTeams(TMap<FString, AINSTeamInfo*>& OutTeams);

	/**bTeamDamageAllowed setter*/
	virtual void SetAllowTeamDamage(bool bTeamDamageAllowed) { bAllowTeamDamage = bTeamDamageAllowed; }

	/** allow if this game allows team damage */
	inline virtual bool GetAllowTeamDamage()const { return bAllowTeamDamage; }

	/**TeamDamageModifier getter*/
	inline virtual float GetTeamDamageModifier()const { return TeamDamageModifier; }

	/**bAllowFallingDamage getter*/
	inline virtual bool GetAllowFallingDamage()const { return bAllowFallingDamage; }

	/**MaxSingleTeamPlayers getter*/
	inline virtual uint8 GetMaxSingleTeamPlayers()const { return MaxSingleTeamPlayers; }

	/** return if allow fire */
	inline virtual bool GetIsAllowFire()const { return bAllowFire; }

	/** return if allow move */
	inline virtual bool GetIsAllowMove()const { return bAllowMove; }

	inline virtual uint8 GetMatchPrepareRemainingTime()const { return MatchPrepareRemainingTime; }

	virtual void SetMatchPrepareRemainingTime(uint8 PrepareTimeRemaining) { MatchPrepareRemainingTime = PrepareTimeRemaining; }

	inline virtual bool GetIsPreparingMatch()const { return bIsMatchPrepare; }

	virtual void SetIsPreparingMatch(bool NewState) { bIsMatchPrepare = NewState; }

	inline virtual bool GetIsPreparingStateFinished()const { return bMatchPreparingFinished; }

	virtual void SetEndPrepringState(bool NewState) { bMatchPreparingFinished = NewState; }

	inline float GetMatchPrepareTime()const { return MatchPrepareTime; }

	inline float GetMatchPrepareTimeRemaining()const { return MatchPrepareRemainingTime; }

	/** return if a damage is cause by team members */
	virtual bool GetIsTeamDamage(class AController* DamageInstigator, class AController* Victim);

	/** get if players should drop their weapon after dead */
	virtual bool GetShouldDropWeaponWhenPlayerDead()const { return bShouldDropWeaponWhenPlayerDead; }

	/** get game mode random available weapon class */
	virtual  UClass* GetRandomGameModeWeaponClass()const;
};
