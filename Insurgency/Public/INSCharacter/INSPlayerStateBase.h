// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "INSPlayerStateBase.generated.h"

class AINSTeamInfo;
class AINSPlayerController;
class AINSWeaponBase;

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
 * Player state
 */
UCLASS()
class INSURGENCY_API AINSPlayerStateBase : public APlayerState
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_TeamInfo, Category = "TeamInfo")
		AINSTeamInfo* PlayerTeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_MyScore, Category = "MyScore")
		int32 MyScore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Repawn")
		float RespawnRemainingTime;

	/** bandwidth save version to replicate  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_RespawnRemainingTime, Category = "Repawn")
		uint8 ReplicatedRespawnRemainingTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Respawn")
		uint8 bIsWaitingForRespawn : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CachedDamageinfo")
		uint8 CachedDamageInfoMaxSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CachedDamageinfo")
		TArray<FCachedDamageInfo> CachedDamageInfos;

	/** indicate how many plays does this player kill */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Kills")
		int32 Kills;

	/** indicate how many death happens on this player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Deaths")
		int32 Death;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	virtual void OnRep_Score()override;
	virtual void BeginPlay()override;

	UFUNCTION()
		virtual void OnRep_TeamInfo();

	UFUNCTION()
		virtual void OnRep_MyScore();

	FTimerHandle RespawnTimer;

	UFUNCTION()
		virtual void OnRep_RespawnRemainingTime();

public:
	virtual void SetPlayerTeam(class AINSTeamInfo* NewTeam);
	virtual AINSTeamInfo* GetPlayerTeam()const { return PlayerTeam; }
	virtual void SetRepawnRemainingTime(float NewTime) { RespawnRemainingTime = NewTime; }
	virtual float GetRespawnRemainingTime()const { return RespawnRemainingTime; }
	virtual void UpdateRepliatedRespawnRemaingTime();
	virtual bool GetIsWaitingForRespawn()const { return bIsWaitingForRespawn; }
	virtual void SetWaitingForRespawn(bool NewState) { bIsWaitingForRespawn = NewState; }
	virtual uint8 GetReplicatedRespawnRemainingTime()const { return ReplicatedRespawnRemainingTime; }
	virtual void ReceiveHitInfo(const struct FTakeHitInfo TakeHitInfo);
	inline int32 GetNumKills()const { return Kills; }
	inline int32 GetNumDeaths()const { return Death; }
	inline void AddKill(int32 KillNum = 1) { Kills += KillNum; }
	inline void AddDeath() { Death += 1; };
	inline float GetKDRation()const { return Kills / Death; }
	virtual void PlayerScore(int32 ScoreToAdd);

	UFUNCTION()
		virtual void OnPlayerKill(class APlayerState* Killer, class APlayerState* Victim, int32 KillerScore, bool bIsTeamDamage);

	UFUNCTION()
		virtual void OnPlayerDamage(class APlayerState* Killer, class APlayerState* Victim, int32 DamagaCauserScore, bool bIsTeamDamage);
	UFUNCTION()
		virtual void TickRespawnTime();
	virtual void Tick(float DeltaSeconds)override;
	virtual void ReceivePlayerDeath(AINSPlayerController* DeadPlayer);
};
