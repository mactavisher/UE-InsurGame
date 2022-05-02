// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSItemManager.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "INSAssets/INSGameMapsAsset.h"
#include "INSGameInstance.generated.h"

class UINSItemManager;

UCLASS()
class INSURGENCY_API UINSGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemTable")
	UDataTable* ItemDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemTable")
	UDataTable* WeaponDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GameMaps")
	UINSGameMapsAsset* GameMaps;

	UPROPERTY()
	TSubclassOf<UINSItemManager> ItemManagerClass;

	UPROPERTY()
	UINSItemManager* ItemManager;

	virtual void Init() override;

public:
	UFUNCTION(BlueprintCallable)
	virtual UDataTable* GetWeaponDataTable() const { return WeaponDataTable; }

	UFUNCTION(BlueprintCallable)
	virtual UDataTable* GetItemDataTable() const { return ItemDataTable; }

	UFUNCTION(BlueprintCallable)
	virtual UINSItemManager* GetItemManager() const { return ItemManager; }
};
