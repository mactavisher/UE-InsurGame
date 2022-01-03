// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "INSMenuInfo.generated.h"

/**
 * defines the main menu table rows
 */
USTRUCT(BlueprintType)
struct FMainMenuInfoRow:public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	/** the menu name*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName  MenuName;

	/** the target widget switch to when the menu is clicked*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName TargetWidgetName;
};



/**
 * struct to hold actual main menu data
 */
USTRUCT(BlueprintType)
struct FMainMenuInfo
{
	GENERATED_USTRUCT_BODY()
	/** the menu name*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName  MenuName;

	/** the target widget switch to when the menu is clicked*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName TargetWidgetName;
};
