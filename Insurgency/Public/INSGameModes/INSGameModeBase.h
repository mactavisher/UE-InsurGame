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

	/** default weapon class that will be used in this game */
	UPROPERTY(EditAnywhere, NoClear, BlueprintReadOnly, Category = Classes)
		TSubclassOf<AINSWeaponBase> DefaultWeaponClass;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,Category="GameModeConfig")
	    float DefaultRestartTime;

	/** indicate whether player should drop their weapon as a pick up,will be replicated via game state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Config)
		uint8 bShouldDropWeaponWhenPlayerDead : 1;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Team")
	    AINSTeamInfo* TerroristTeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
		AINSTeamInfo* CTTeam;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="GameState")
	    uint8 bIsMatchPrepare:1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameState")
		uint8 bMatchPreparingFinished : 1;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="PrepareTime")
	    float MatchPrepareTime;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="PreParingTime")
	    float MatchPrepareRemainingTime;

	UPROPERTY()
		FTimerHandle MatchPrepareTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestWeaponClasses")
		TArray<TSubclassOf< AINSWeaponBase>> AvailableWeaponsClasses; 

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual bool HasMatchStarted() const override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)override;
	virtual void PreInitializeComponents()override;
	virtual void SpawnTerrorisTeam();
	virtual void SpawnCounterTerroristTeam();
	virtual void PlayerScore(AINSPlayerController* ScoringPlayer);
	virtual void EndMatchPerparing();
	virtual void AssignPlayerTeam(class AINSPlayerController* NewPlayer);
	virtual void Tick(float DeltaSeconds)override;
	virtual void ScorePlayer(class AINSPlayerController* PlayerToScore, int32 Score);
	UFUNCTION()
	virtual void CountDownMatchPrepare();
public:
	/**Modify damage according to the game rules*/
	virtual void ModifyDamage(float& OutDamage, class AController* PlayerInstigator, class AController* Victim,FName BoneName);

	/**bTeamDamageAllowed setter*/
	virtual void SetAllowTeamDamage(bool bTeamDamageAllowed) { bAllowTeamDamage = bTeamDamageAllowed; }

	/**confirms a player kill*/
	virtual void ConfirmKill(class AController* Killer, class AController* Victim);

	/**bTeamDamageAllowed getter*/
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

	/**OutTeams getter*/
	virtual void GetInGameTeams(TMap<FString, AINSTeamInfo*>& OutTeams);

	/** return if a damage is cause by team members */
	virtual bool GetIsTeamDamage(class AController* DamageInstigator, class AController* Victim);

	virtual bool GetShouldDropWeaponWhenPlayerDead()const { return bShouldDropWeaponWhenPlayerDead; }

	virtual  UClass* GetRandomWeapon()const;

};
