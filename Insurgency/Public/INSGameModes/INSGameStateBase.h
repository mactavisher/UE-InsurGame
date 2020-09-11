// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Insurgency/Insurgency.h"
#include "GameFramework/GameState.h"
#include "INSGameStateBase.generated.h"

class AINSTeamInfo;
class USoundCue;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSGameState, Log, All);

/**
 *   Replicated version of Important data of GameMode,
 *   game mode only exists on server side
 *   each client need to know those informations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnKillSignature, class APlayerState*, KillerState, class APlayerState*, VictimState, int32, KillerScore, bool, bIsTeamDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageSignature, class APlayerState*, KillerState, class APlayerState*, VictimState, int32, DamagaCauserScore, bool, bIsTeamDamage);
UCLASS()
class INSURGENCY_API AINSGameStateBase : public AGameState
{
	GENERATED_UCLASS_BODY()

protected:
	/** is fire a weapon allowed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "GameModeConfig")
		uint8 bAllowFire : 1;

	/** is moving allowed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "GameModeConfig")
		uint8 bAllowMove : 1;

	/** is moving allowed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_GameType, Category = "GameModeConfig")
		EGameType CurrentGameType;

	/** how much time players have to wait before re-spawn */
	UPROPERTY(Replicated)
		uint8 ReSpawnTime;

	/** Terrorist Team */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Team")
		AINSTeamInfo* TerroristTeam;

	/** Counter Terrorist Team */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Team")
		AINSTeamInfo* CTTeam;

	/** replicated version of Game modes,indicate whether player should drop their weapon as a pick up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = Config)
		uint8 bShouldDropWeaponWhenPlayerDead : 1;

	/** replicated game state version that all connected clients should know */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_PrepareMath, Category = "GameState")
		uint8 bIsMatchPrepare : 1;

	/** replicated game state version that all connected clients should know */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_FinishePreparing, Category = "GameState")
		uint8 bMatchPrepareFinished : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "MatchPreparingTime")
		float MatchPrepareTime;

	/** replicated game state version that all connected clients should know */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_PreparingRemainingTime, Category = "MatchPreparingTime")
		uint8 ReplicatedMatchPrepareRemainingTime;

	/** replicated game state version that all connected clients should know */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "MatchPreparingTime")
		float GameDefaultRespawnTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI Sound")
		USoundCue* ClockTickingSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI Sound")
		USoundCue* GameModeSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI Sound")
		USoundCue* GameBGMSound;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
		FOnKillSignature OnKill;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
		FOnDamageSignature OnDamage;


protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps)const override;

	virtual void Tick(float DeltaSeconds)override;

	UFUNCTION()
		virtual void OnRep_PrepareMath();

	/**
	 * @desc send game kill event to all connected clients
	 * @param Killer        the killer player
	 * @param Victim        the victim Player
	 * @param KillerScore   the killer score
	 * @param bIsTeamDamage is this damage caused by team
	 */
	UFUNCTION(Client, NetMulticast, Unreliable, WithValidation)
		virtual void ClientsReceiveKillEvent(class APlayerState* Killer, class APlayerState* Victim, int32 KillerScore, bool bIsTeamDamage);

	UFUNCTION()
		virtual void OnRep_FinishePreparing();

	UFUNCTION()
		virtual void OnRep_PreparingRemainingTime();

	UFUNCTION()
		virtual void OnRep_GameType();

public:
	inline virtual bool GetAllowFire()const { return bAllowFire; }

	/**
	 * @desc   Server Only   when kills happen in game
	 * @param  Killer        the killer player
	 * @param  Victim        the killed Player
	 * @param  KillerScore   the killer score
	 * @param  bIsTeamDamage Is this a team caused kill
	 */
	virtual void OnPlayerKilled(class AController* Killer, class AController* Victim, int32 KillerScore, bool bIsTeamDamage);

	/**
	 * @desc   Server Only   when kills happen in game
	 * @param  Killer        the DamageInstigator
	 * @param  Victim        the Victim Player
	 * @param  KillerScore   the Damage Amount Caused
	 * @param  bIsTeamDamage Is this a team caused damage
	 */
	virtual void OnPlayerDamaged(class AController* DamageInstigtor, class AController* Victim, float DamageAmount, bool bIsTeamDamage);

	/** is allow move currently */
	inline virtual bool GetAllowMove()const { return bAllowMove; }

	/** is game in prepare  */
	inline virtual bool GetIsPreparingMatch()const { return bIsMatchPrepare; }

	/**
	 * @desc   Server Only   set if the Match should be Prepare state
	 * @param  NewState      is Prepare state
	 */
	virtual void SetPreparingMatch(bool NewState) { bIsMatchPrepare = NewState; }

	/** is prepare match state finished */
	inline virtual bool GetIsPreparingStateFinished()const { return bMatchPrepareFinished; }

	/**
	 * @desc   Server Only   set end Prepare state
	 * @param  NewState      is Prepare state
	 */
	virtual void SetEndPrepringState(bool NewState) { bMatchPrepareFinished = NewState; }


	/** Get how much time we still have to wait before we can finish prepare state and actually can play */
	inline virtual uint8 GetReplicatedMatchPrepareRemainingTime()const { return ReplicatedMatchPrepareRemainingTime; }

	/**
	 * @desc   Server Only   set how much time we still have to wait before we can finish prepare state and actually can play
	 * @param  PrepareTimeRemaining      the prepare time still remaining
	 */
	virtual void SetMatchPrepareRemainingTime(uint8 PrepareTimeRemaining);

	/** return the current game type */
	inline virtual EGameType GetCurrentGameType()const { return CurrentGameType; }

	/**
	 * @desc   Server Only     set allow move state,and the state should be sent to all the clients and server should validate this
	 * @param  bIsAllowed      is allow move now
	 */
	virtual void SetAllowMove(bool bIsAllowed) { bAllowMove = bIsAllowed; }

	/**
	 * @desc   Server Only     set allow fire state,and the state should be sent to all the clients and server should validate this
	 * @param  bIsAllowed      is allow fire now
	 */
	virtual void SetAllowFire(bool bIsAllowed) { bAllowFire = bIsAllowed; }

	/**
	 * @desc   Server Only        set how much time player waiting for a re-spawn after killed
	 * @param  NewRespawnTime     re-spawn waiting time
	 */
	virtual void SetRespawnTime(uint8 NewRespawnTime) { this->ReSpawnTime = NewRespawnTime; }

	virtual uint8 GetRespawnTime()const { return ReSpawnTime; }

	/**
	 * @desc   Server Only        Set the Terrorist Team info
	 * @param  NewTerroTeam       Terrorist Team to set
	 */
	virtual void SetTerroristTeam(class AINSTeamInfo* NewTerroTeam) { this->TerroristTeam = NewTerroTeam; }

	/** return the terrorist team info */
	inline virtual class AINSTeamInfo* GetTerroristTeamInfo()const { return TerroristTeam; }

	/**
	 * @desc   Server Only        Set the Terrorist Team info
	 * @param  NewCTTeam          CT Team to set
	 */
	virtual void SetCTTeam(class AINSTeamInfo* NewCTTeam) { this->CTTeam = NewCTTeam; }

	/** return the CT team info */
	inline virtual class AINSTeamInfo* GetCTTeamInfo()const { return CTTeam; }

	virtual void SetShouldDropWeaponWhenPlayerDead(bool ShouldDropWeapon) { bShouldDropWeaponWhenPlayerDead = ShouldDropWeapon; }

	inline virtual bool GetShouldDropWeaponWhenPlayerDead()const { return bShouldDropWeaponWhenPlayerDead; }

	virtual float GetGameDefaultRespawnTime()const { return GameDefaultRespawnTime; }

	virtual void SetGameDefaultRespawnTime(float NewRespawnTime) { GameDefaultRespawnTime = NewRespawnTime; }

};
