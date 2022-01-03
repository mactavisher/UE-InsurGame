// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSAssets/INSWeaponAssets.h"
#include "UObject/NoExportTypes.h"
#include "INSItemManager.generated.h"

class UINSGameInstance;
class UDataTable;
class AINSItems;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSItemManager, Log, All);
/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSItemManager : public UObject
{
	GENERATED_UCLASS_BODY()
protected:
	/** weapon table which stores the weapon data*/
	UPROPERTY()
	UDataTable* ItemDataTable;

	/** the owing game instance*/
	UPROPERTY()
	UINSGameInstance* OwningGameInstance;

public:
	
	/**
	 * sets the weapon table data by the owing game instance
	 * @param NewItemDataTable the weapon data table to set
	 */
	virtual void SetItemTable(UDataTable* NewItemDataTable);

	UFUNCTION(BlueprintCallable)
	virtual UDataTable* GetItemDataTable()const{return ItemDataTable;}

	/**
	 * sets the owing game instance
	 * @param NewGameInstance the owing game instance
	 */
	virtual void SetOwingGameInstance(UINSGameInstance* NewGameInstance);
	
	virtual void GetItemById(int32 ItemId, FWeaponInfoData& OutWeaponInfo);

	virtual void InitItemData(AINSItems* InItem);

	virtual void GetAllItemInfos(TArray<FItemInfoData> InItemInfos);

	virtual void GetAllWeaponsInfos(TArray<FWeaponInfoData>& InWeaponInfos);
};
