// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "Insurgency/Insurgency.h"
#include "INSLocalPlayer.generated.h"

/**
 *
 */
UCLASS()
class INSURGENCY_API UINSLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Prefrence")
		ETeamType PrefTeam;
	virtual ETeamType GetPrefTeamType()const { return PrefTeam; }
	virtual void SetPrefTeamType(ETeamType NewPrefTeam) { PrefTeam = NewPrefTeam; }
};
