// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSAssets/INSWeaponAssets.h"
#include "UObject/NoExportTypes.h"
#include "INSItemManager.generated.h"

class UINSGameInstance;
class UDataTable;
class AINSItems;
class APawn;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSItemManager, Log, All);

/**
 *  a Item manager is responsible to manage the items in game
 */
UCLASS()
class INSURGENCY_API UINSItemManager : public UObject
{
	GENERATED_UCLASS_BODY()
protected:
	/** weapon table which stores the weapon data*/
	UPROPERTY(BlueprintReadOnly)
	UDataTable* ItemDataTable;

	UPROPERTY(BlueprintReadOnly)
	UDataTable* WeaponDataTable;

	/** the owing game instance*/
	UPROPERTY(BlueprintReadOnly)
	UINSGameInstance* OwningGameInstance;

public:
	/**
	 * sets the weapon table data by the owing game instance
	 * @param NewItemDataTable the weapon data table to set
	 */
	UFUNCTION(BlueprintCallable)
	virtual void SetItemTable(UDataTable* NewItemDataTable);
	UFUNCTION(BlueprintCallable)
	virtual void SetWeaponTable(UDataTable* NewWeaponTable);

	UFUNCTION(BlueprintCallable)
	virtual UDataTable* GetItemDataTable() const { return ItemDataTable; }

	UFUNCTION(BlueprintCallable)
	virtual UDataTable* GetWeaponDataTable() const { return WeaponDataTable; }

	/**
	 * sets the owing game instance
	 * @param NewGameInstance the owing game instance
	 */
	virtual void SetOwingGameInstance(UINSGameInstance* NewGameInstance);

	/**
	 * get the item info by given itemId
	 * @param ItemId the item id to find
	 * @param OutItemInfo out put the item info
	 */
	virtual void GetWeaponItemById(int32 ItemId, FWeaponInfoData& OutItemInfo);

	virtual void GetItemById(int32 ItemId, FItemInfoData& OutItemInfo);

	virtual void InitItemData(AINSItems* InItem);

	virtual AINSItems* CreateItemInstance(int32 ItemId, const FTransform& InitialTransform, AActor* Owner, APawn* Instigator);

	virtual AINSWeaponBase* CreateWeaponItemInstance(int32 ItemId, const FTransform& InitialTransform, AActor* Owner, APawn* Instigator, const uint8 InventorySlotIndex);

	virtual void GetAllItemInfos(TArray<FItemInfoData> InItemInfos);

	virtual void GetAllWeaponsInfos(TArray<FWeaponInfoData>& InWeaponInfos);
};
