// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "INSPlayerStateBase.generated.h"

class AINSTeamInfo;
class AINSPlayerController;

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
 *
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

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="CachedDamageinfo")
	   TArray<FCachedDamageInfo> CachedDamageInfos;


protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps)const override;
	virtual void OnRep_Score()override;

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
	UFUNCTION()
		virtual void TickRespawnTime();
	virtual void Tick(float DeltaSeconds)override;
	virtual void ReceivePlayerDeath(AINSPlayerController* DeadPlayer);
};
