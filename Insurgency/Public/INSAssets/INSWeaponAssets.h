// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Insurgency/Insurgency.h"
#include "INSWeaponAssets.generated.h"

struct FItemTableRows;
class AINSWeaponBase;
class UAnimMontage;
class UBlendSpace;
class UAnimationAsset;
class UTexture2D;
class AINSItems;

USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	/** the item unique id */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 ItemId;

	/** the path for this item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<AINSItems> ItemClass;

	/** icon image path for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FCanvasIcon ItemIconAsset;

	/** icon image path for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ItemTextureAsset;

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
struct FWeaponTableRow : public FItemTableRow
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EWeaponReloadType WeaponReloadType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 Priority;

	/**single clip ammo capacity for this item if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 BaseClipCapacity;

	/**max ammo can carry this item if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 BaseClipSize;

	/**time between each shot if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MuzzleVelocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ScanTraceRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bNeedOpticRail:1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BaseAimTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RecoilHorizontalBase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RecoilVerticalBase;
};

USTRUCT(BlueprintType)
struct FItemInfoData
{
	GENERATED_USTRUCT_BODY()

	virtual ~FItemInfoData()
	{
	}

	/** the item unique id */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 ItemId;

	/** the path for this item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<AINSItems> ItemClass;

	/** icon image path for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FCanvasIcon ItemIconAsset;

	/** icon image path for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ItemTextureAsset;

	/** item type*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EItemType ItemType;

	/** item Name*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName ItemName;

	/** item simple description*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName Desc;

public:
	/**
	 * copy the struct properties from a given Item table
	 * @param ItemTableRow the item table row info to copy from
	 */
	virtual void CopyDataFromTable(FItemTableRow* ItemTableRow)
	{
		ItemId = ItemTableRow->ItemId;
		ItemType = ItemTableRow->ItemType;
		ItemClass = ItemTableRow->ItemClass;
		ItemIconAsset = ItemTableRow->ItemIconAsset;
		ItemTextureAsset = ItemTableRow->ItemTextureAsset;
		ItemName = ItemTableRow->ItemName;
		Desc = ItemTableRow->Desc;
	}
};

USTRUCT(BlueprintType)
struct FWeaponInfoData : public FItemInfoData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EWeaponReloadType WeaponReloadType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 Priority;

	/**single clip ammo capacity for this item if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 BaseClipCapacity;

	/**base clip size can carry*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 BaseClipSize;

	/**time between each shot if it's a weapon*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MuzzleVelocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ScanTraceRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bNeedOpticRail:1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BaseAimTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RecoilHorizontalBase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RecoilVerticalBase;
	virtual void CopyDataFromTable(FItemTableRow* ItemTableRow) override;
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
