// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "Engine/DataAsset.h"
#include "Insurgency/Insurgency.h"
#include "INSGameMapsAsset.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FGameMapInfo
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName MapName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowedClasses="World"))
	FSoftObjectPath EntryMap;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FCanvasIcon MapIcon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FCanvasIcon MiniMapIcon;
};

USTRUCT(BlueprintType)
struct FGameModeAndMapsInfo
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EGameModeEnum GameMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FGameMapInfo> Maps;
};

UCLASS()
class INSURGENCY_API UINSGameMapsAsset : public UDataAsset
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GameMaps")
	TArray<FGameModeAndMapsInfo> GameMaps;
};
