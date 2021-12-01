// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "INSItems/INSItems.h"
#include "Insurgency/Insurgency.h"
#include "INSWeaponAssets.generated.h"

class AINSWeaponBase;
class UAnimMontage;
class UBlendSpace;
class UAnimationAsset;
class UTexture2D;


USTRUCT(BlueprintType)
struct FItemInfoData
{
	GENERATED_USTRUCT_BODY()

	/** the item unique id */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 ItemId;

	/** the path for this item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AINSItems> ItemClass;

	/** icon image path for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FCanvasIcon ItemIconAsset;

	/** icon image path for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* ItemTextureAsset;

	/** item type*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EItemType ItemType;

	/** item Name*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName ItemName;

	/** item simple description*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName Desc;
};

USTRUCT(BlueprintType)
struct FWeaponInfoData : public FItemInfoData
{
	GENERATED_USTRUCT_BODY()

	/**single clip ammo capacity for this item if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 BaseClipCapacity;

	/**max ammo can carry this item if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 MaxAmmoCapacity;

	/**time between each shot if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float MuzzleVelocity;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float ScanTraceRange;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	uint8 bNeedOpticRail:1;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float BaseAimTime;
};

USTRUCT(BlueprintType)
struct FWeaponTableRows:public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	/** the item unique id */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 ItemId;

	/** the path for this item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<AINSWeaponBase> ItemClass;

	/** icon image path for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FCanvasIcon ItemIconAsset;

	/** icon image path for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<UTexture2D> ItemTextureAsset;

	/** item type*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EItemType ItemType;

	/** item Name*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName ItemName;

	/** item simple description*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName Desc;

	/**single clip ammo capacity for this item if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 BaseClipCapacity;

	/**max ammo can carry this item if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 MaxAmmoCapacity;

	/**time between each shot if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float MuzzleVelocity;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float ScanTraceRange;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	uint8 bNeedOpticRail:1;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float BaseAimTime;
};

USTRUCT(BlueprintType)
struct FWeaponAttachmentInfoData : public FItemInfoData
{
	GENERATED_USTRUCT_BODY()
	
};


UCLASS()
class INSURGENCY_API UINSWeaponAssets : public UDataAsset
{
	GENERATED_BODY()
};
