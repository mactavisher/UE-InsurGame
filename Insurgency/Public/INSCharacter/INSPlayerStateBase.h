// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "INSPlayerStateBase.generated.h"

class AINSTeamInfo;
class AINSPlayerController;
class AINSWeaponBase;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSPlayerState, Log, All);

USTRUCT(BlueprintType)
struct FCachedDamageInfo
{
	GENERATED_USTRUCT_BODY()
public:
	/** player indicate this damage */
	TWeakObjectPtr<AController> DamageIndicator;

	/** actual actor cause this damage */
	TWeakObjectPtr<AActor> DamageCauser;

	/** Weapon cause this damage */
	TWeakObjectPtr<AINSWeaponBase> DamageWeapon;

	/** actual damage caused */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float LastDamageAmount;
public:
	FCachedDamageInfo()
		: DamageIndicator(nullptr)
		  , DamageCauser(nullptr)
		  , DamageWeapon(nullptr)
		  , LastDamageAmount(0.f)
	{
	}
};

/**
 * struct to track combo kill
 */
USTRUCT(BlueprintType)
struct FComboKillInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	int32 CombokillCount;

	UPROPERTY()
	float LastKillTime;

	UPROPERTY()
	float ComboBreakTime;

public:
	FComboKillInfo()
		: CombokillCount(0)
		  , LastKillTime(0.f)
		  , ComboBreakTime(5.f)
	{
	}
};


/**
 * Player state
 */
UCLASS()
class INSURGENCY_API AINSPlayerStateBase : public APlayerState
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_TeamInfo, Category = Team)
	AINSTeamInfo* PlayerTeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerState)
	float RespawnRemainingTime;

	/** bandwidth save version to replicate  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_RespawnRemainingTime, Category = PlayerState)
	uint8 ReplicatedRespawnRemainingTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = PlayerState)
	uint8 bIsWaitingForRespawn : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Damage)
	uint8 CachedDamageInfoMaxSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Damage)
	TArray<FCachedDamageInfo> CachedDamageInfos;

	/** indicate how many plays does this player kill */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = "OnRep_Kills", Category = PlayerState)
	int32 Kills;

	/** indicate how many player does this player kill by mistake,kill friendlies typically */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = "OnRep_MistakeKill", Category = PlayerState)
	int32 MissTakeKill;

	/** indicate how many death happens on this player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = "OnRep_Deaths", Category = PlayerState)
	int32 Deaths;

	/** k/d ratio,not this is NOT replicated so client will need to update this it self  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerState)
	float KDRatio;

	UPROPERTY()
	FComboKillInfo ComboKillInfo;

protected:
	//~ Begin AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor interface

	//~ Begin APlayerState interface
	virtual void OnRep_Score() override;
	//~ End APlayerState interface

	/** rep callbacks */
	UFUNCTION()
	virtual void OnRep_TeamInfo();

	UFUNCTION()
	virtual void OnRep_Deaths();

	UFUNCTION()
	virtual void OnRep_Kills();

	UFUNCTION()
	virtual void OnRep_MistakeKill();

	UFUNCTION()
	virtual void OnRep_RespawnRemainingTime();

	/** re_spawn timer after kill */
	FTimerHandle RespawnTimer;

	/** re_spawn timer callback */
	UFUNCTION()
	virtual void TickRespawnTime();

public:
	/**
	 * @Desc  Set the player team
	 * @Param NewTeam the team set for this player
	 */
	virtual void SetPlayerTeam(class AINSTeamInfo* NewTeam);

	/**
	 * @Desc  Set the re_spawn remaining time for this player if killed
	 * @Param NewTime time to set
	 */
	virtual void SetRespawnRemainingTime(const float NewTime) { RespawnRemainingTime = NewTime; }

	/**
	 * @Desc  Set the player waiting for re_spawn state
	 * @Param NewState new state to set
	 */
	virtual void SetWaitingForRespawn(const bool NewState) { bIsWaitingForRespawn = NewState; }

	/**
	 * @Desc  add the score to the current score so score gets accumulated
	 * @Param ScoreToAdd  score to add
	 */
	virtual void AddScore(const float ScoreToAdd);

	/**
	 * @Desc  Add kill num
	 * @Param KillNum kills to add
	 */
	inline void AddKill(const int32 KillNum = 1);

	/**
	 * @Desc  Add mistake kill num, such as kill friendlies
	 * @Param mistake kills to add
	 */
	inline void AddMissTakeKill(const int32 KillsToAdd = 1);

	/**
	 * @Desc  called when the pawn is dead
	 */
	virtual void OnPawnCharDeath();

	/**
	 * @Desc Add deaths num
	 * @Param DeathToAdd deaths to add ,default is 1
	 */
	inline void AddDeath(const int32 DeathToAdd = 1);

	/** gets the current player team */
	virtual AINSTeamInfo* GetPlayerTeam() const { return PlayerTeam; }

	/** gets the re_spawn remaining time */
	virtual float GetRespawnRemainingTime() const { return RespawnRemainingTime; }

	/** ticking and update the remaining re_spawn time */
	virtual void UpdateReplicatedRespawnRemainingTime();

	/** get whether this player is waiting for a re_spawn */
	virtual bool GetIsWaitingForRespawn() const { return bIsWaitingForRespawn; }

	/** gets the replicated re_spawn remaining time */
	virtual uint8 GetReplicatedRespawnRemainingTime() const { return ReplicatedRespawnRemainingTime; }

	/** gets the kills of this player */
	inline int32 GetNumKills() const { return Kills; }

	/** gets the deaths of this player */
	inline int32 GetNumDeaths() const { return Deaths; }

	/** returns the current K/D Ratio */
	inline float GetKDRatio() const { return KDRatio; }

	/** update the k/d ration after kill or death */
	virtual void UpdateKDRatio();
};
