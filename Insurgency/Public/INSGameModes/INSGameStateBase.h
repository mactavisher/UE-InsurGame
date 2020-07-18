// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Insurgency/Insurgency.h"
#include "GameFramework/GameState.h"
#include "INSGameStateBase.generated.h"

class AINSTeamInfo;
class USoundCue;
/**
 *   Replicated version of Important data of GameMode,
 *   game mode only exists on server side
 *   each client need to know those informations
 */
UCLASS()
class INSURGENCY_API AINSGameStateBase : public AGameState
{
	GENERATED_UCLASS_BODY()

protected:
	/** is fire a weapon allowed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated, Category = "GameModeConfig")
		uint8 bAllowFire : 1;

	/** is moving allowed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "GameModeConfig")
		uint8 bAllowMove : 1;

	/** is moving allowed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated,ReplicatedUsing=OnRep_GameType, Category = "GameModeConfig")
		EGameType CurrentGameType;

	/** how much time players have to wait before re-spawn */
	UPROPERTY(Replicated)
	   uint8 ReSpawnTime;

	/** Terrorist Team */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated, Category = "Team")
		AINSTeamInfo* TerroristTeam;

	/** Counter Terrorist Team */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated,Category = "Team")
		AINSTeamInfo* CTTeam;

	/** replicated version of Game modes,indicate whether player should drop their weapon as a pick up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated,Category = Config)
		uint8 bShouldDropWeaponWhenPlayerDead : 1;

	/** replicated game state version that all connected clients should know */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated,ReplicatedUsing=OnRep_PrepareMath, Category = "GameState")
		uint8 bIsMatchPrepare : 1;

	/** replicated game state version that all connected clients should know */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated,ReplicatedUsing=OnRep_FinishePreparing, Category = "GameState")
		uint8 bMatchPrepareFinished: 1;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Replicated,Category="MatchPreparingTime")
        float MatchPrepareTime;

	/** replicated game state version that all connected clients should know */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated,ReplicatedUsing=OnRep_PreparingRemainingTime,Category = "MatchPreparingTime")
		uint8 ReplicatedMatchPrepareRemainingTime;

	/** replicated game state version that all connected clients should know */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated, Category = "MatchPreparingTime")
		float GameDefaultRespawnTime;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="UI Sound")
	    USoundCue* ClockTickingSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI Sound")
	    USoundCue* GameModeSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI Sound")
		USoundCue* GameBGMSound;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps)const override;

	virtual void Tick(float DeltaSeconds)override;

	UFUNCTION()
	virtual void OnRep_PrepareMath();

	UFUNCTION()
	virtual void OnRep_FinishePreparing();

	UFUNCTION()
	virtual void OnRep_PreparingRemainingTime();

	UFUNCTION()
	virtual void OnRep_GameType();

public:
	inline virtual bool GetAllowFire()const { return bAllowFire; }

	inline virtual bool GetAllowMove()const { return bAllowMove;}

	inline virtual bool GetIsPreparingMatch()const { return bIsMatchPrepare; }

	virtual void SetPreparingMatch(bool NewState) { bIsMatchPrepare = NewState; }

	inline virtual bool GetIsPreparingStateFinished()const { return bMatchPrepareFinished; }

	virtual void SetEndPrepringState(bool NewState) { bMatchPrepareFinished = NewState; }

	inline virtual uint8 GetReplicatedMatchPrepareRemainingTime()const { return ReplicatedMatchPrepareRemainingTime; }

	virtual void SetMatchPrepareRemainingTime(uint8 PrepareTimeRemaining) { ReplicatedMatchPrepareRemainingTime = PrepareTimeRemaining; }

	inline virtual EGameType GetCurrentGameType()const { return CurrentGameType; }

	virtual void SetAllowMove(bool bIsAllowed) { bAllowMove = bIsAllowed; }

	virtual void SetAllowFire(bool bIsAllowed) { bAllowFire = bIsAllowed; }

	virtual void SetRespawnTime(uint8 NewRespawnTime) { this->ReSpawnTime = NewRespawnTime; }
	virtual uint8 GetRespawnTime()const { return ReSpawnTime; }

	virtual void SetTerroristTeam(class AINSTeamInfo* NewTerroTeam) { this->TerroristTeam = NewTerroTeam; }

	inline virtual class AINSTeamInfo* GetTerroristTeamInfo()const { return TerroristTeam; }

	virtual void SetCTTeam(class AINSTeamInfo* NewCTTeam) { this->CTTeam = NewCTTeam; }

	inline virtual class AINSTeamInfo* GetCTTeamInfo()const { return CTTeam; }

	virtual void SetShouldDropWeaponWhenPlayerDead(bool ShouldDropWeapon) { bShouldDropWeaponWhenPlayerDead = ShouldDropWeapon; }

	inline virtual bool GetShouldDropWeaponWhenPlayerDead()const { return bShouldDropWeaponWhenPlayerDead; }

	virtual float GetGameDefaultRespawnTime()const { return GameDefaultRespawnTime; }

	virtual void SetGameDefaultRespawnTime(float NewRespawnTime) { GameDefaultRespawnTime = NewRespawnTime; }

};
