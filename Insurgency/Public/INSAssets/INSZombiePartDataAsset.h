// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "INSZombiePartDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FZombiePart
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="parts")
	TMap<int32, USkeletalMesh*> headPart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="parts")
	TMap<int32, USkeletalMesh*> TorsorPart;
};

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSZombiePartDataAsset : public UDataAsset
{
	GENERATED_BODY()
};
