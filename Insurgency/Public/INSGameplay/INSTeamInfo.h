// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Insurgency/Insurgency.h"
#include "INSTeamInfo.generated.h"

class AINSPlayerController;
class AINSPlayerStateBase;


/**
 *  Team Info class for Team Based Game Modes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTeamTypeChangedSignature);

UCLASS(Blueprintable)
class INSURGENCY_API AINSTeamInfo : public AInfo
{
	GENERATED_UCLASS_BODY()
	/** current team type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_TeamType, Category = "TeamType")
	ETeamType ThisTeamType;

	/** uint8 that limit a team maximum member to 255 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
	TArray<AINSPlayerStateBase*> TeamMembers;

	/** this team's score */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Scoring")
	float TeamScore;

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FTeamTypeChangedSignature OnTeamTypeChange;

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~end AActor interface
public:
	/**
	 * @desc Set team type for this team
	 * @params NewTeamType  New Team Type to Set
	 */
	virtual void SetTeamType(ETeamType NewTeamType);

	/** return team type */
	virtual ETeamType GetTeamType() const { return ThisTeamType; }

	/** call back function when team type set or changed */
	UFUNCTION()
	virtual void OnRep_TeamType();

	/**
	 * @desc add a new player in this team
	 * @param NewPlayer  New Player to add in
	 */
	virtual void AddPlayerToThisTeam(class AINSPlayerStateBase* NewPlayer);

	/** get current team member size */
	virtual uint8 GetCurrentTeamPlayers() const { return (uint8)TeamMembers.Num(); }

	/** Sort member players by their score */
	virtual void SortPlayersByScore();

	/**
	 * @desc remove a player     from this Team
	 * @param PlayerToRemove     player to remove from this team
	 */
	virtual void RemovePlayer(class AINSPlayerStateBase* PlayerToRemove);

	/** is team maximum members reached ? */
	inline virtual bool isTeamFull();

	/** add team score */
	virtual void AddTeamScore(int32 ScoreToAdd) { TeamScore += FMath::CeilToFloat(ScoreToAdd); };

	/**
	 * retrieve the team score
	 */
	virtual float GetTeamScore() const { return TeamScore; }
};
