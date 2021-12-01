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
	UPROPERTY()
	UDataTable* WeaponDateTable;

	UPROPERTY()
	UINSGameInstance* OwningGameInstance;

public:
	virtual void SetWeaponsTable(UDataTable* WeaponTable);

	virtual void SetOwingGameInstance(UINSGameInstance* NewGameInstance);

	virtual void GetItemById(int32 ItemId, FWeaponInfoData& OutWeaponInfo);

	virtual void InitItemData(AINSItems* InItem);
};
