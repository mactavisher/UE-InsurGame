// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Insurgency/Insurgency.h"
#include "INSTeamInfo.generated.h"

class AINSPlayerController;


/**
 *  Team Info class for Team Based Game Modes
 */
UCLASS(Blueprintable)
class INSURGENCY_API AINSTeamInfo : public AInfo
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Terrorist Team info")
	    ETeamType ThisTeamType;

	/** uint8 that limit a team maximum member to 255 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
		TArray<AINSPlayerController*> TeamMembers;

	/** this team's score */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Scoring")
		float  TeamScore;

protected:

	//~ Begin AActor Interface
	virtual void BeginPlay()override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	//~end AActor interface
public:
	/** assign  team type for this team */
	virtual void SetTeamType(ETeamType NewTeamType) { this->ThisTeamType = NewTeamType; }

	/** return team type */
	virtual ETeamType GetTeamType()const { return ThisTeamType; }

	/** add a player into this team */
	virtual void AddPlayerToThisTeam(class AINSPlayerController* NewPlayer);

	/** get current team member size */
	virtual uint8 GetCurrentTeamPlayers()const { return (uint8)TeamMembers.Num(); }

	/** Sort member players by their score */
	virtual void SortPlayersByScore();
    
	/** remove a player from this team */
	virtual void RemovePlayer(class AINSPlayerController* PlayerToRemove);

	/** is team maximum members reached ? */
	inline virtual bool isTeamFull();

	/** add team score */
	virtual void AddTeamScore(int32 ScoreToAdd) { TeamScore += FMath::CeilToFloat(ScoreToAdd); };
};
