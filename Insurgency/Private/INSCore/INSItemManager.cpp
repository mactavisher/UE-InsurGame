// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCore/INSItemManager.h"
#include "INSCore/INSGameInstance.h"
#include "INSItems/INSItems.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

DEFINE_LOG_CATEGORY(LogINSItemManager);

UINSItemManager::UINSItemManager(const FObjectInitializer& ObjectInitalizer) : Super(ObjectInitalizer)
{
}

void UINSItemManager::SetItemTable(UDataTable* NewItemDataTable)
{
	this->ItemDataTable = NewItemDataTable;
}

void UINSItemManager::SetOwingGameInstance(UINSGameInstance* NewGameInstance)
{
	this->OwningGameInstance = NewGameInstance;
}

void UINSItemManager::GetItemById(int32 ItemId, FWeaponInfoData& OutWeaponInfo)
{
	TArray<FWeaponTableRows*> AllRows;
	ItemDataTable->GetAllRows(TEXT(""), AllRows);
	FWeaponTableRows* MatchedRow = nullptr;
	for (FWeaponTableRows* WeaponRow : AllRows)
	{
		if (WeaponRow->ItemId == ItemId)
		{
			MatchedRow = WeaponRow;
			break;
		}
	}
	if (MatchedRow)
	{
		OutWeaponInfo.BaseDamage = MatchedRow->BaseDamage;
		OutWeaponInfo.MuzzleVelocity = MatchedRow->MuzzleVelocity;
		OutWeaponInfo.BaseClipCapacity = MatchedRow->BaseClipCapacity;
		OutWeaponInfo.MaxAmmoCapacity = MatchedRow->MaxAmmoCapacity;
		OutWeaponInfo.TimeBetweenShots = MatchedRow->TimeBetweenShots;
		OutWeaponInfo.Desc = MatchedRow->Desc;
		//WeaponInfoData.ItemClass = MatchedRow->ItemClass;
		OutWeaponInfo.ItemId = MatchedRow->ItemId;
		OutWeaponInfo.ItemIconAsset = MatchedRow->ItemIconAsset;
		//WeaponInfoData.ItemTextureAsset = MatchedRow->ItemTextureAsset->ClassDefaultObject
	}
	else
	{
		UE_LOG(LogINSItemManager, Warning, TEXT("Item of ItemId:%d not found"), ItemId);
	}
}


void UINSItemManager::InitItemData(AINSItems* InItem)
{
	if (InItem)
	{
		if (InItem->GetItemType() == EItemType::WEAPON)
		{
			AINSWeaponBase* InWeapon = Cast<AINSWeaponBase>(InItem);
			if (InWeapon)
			{
				FWeaponInfoData WeaponInfoData;
				GetItemById(InItem->GetItemId(), WeaponInfoData);
				InWeapon->SetWeaponInfoData(WeaponInfoData);
			}
		}
	}
}

void UINSItemManager::GetAllItemInfos(TArray<FItemInfoData> InItemInfos)
{
}

void UINSItemManager::GetAllWeaponsInfos(TArray<FWeaponInfoData>& InWeaponInfos)
{
	TArray<FWeaponTableRows*> AllRows;
	ItemDataTable->GetAllRows(TEXT(""), AllRows);
	if (AllRows.Num() <= 0)
	{
		return;
	}
	for (const FWeaponTableRows* const WeaponRow : AllRows)
	{
		FWeaponInfoData WeaponInfoData;
		WeaponInfoData.BaseDamage = WeaponRow->BaseDamage;
		WeaponInfoData.MuzzleVelocity = WeaponRow->MuzzleVelocity;
		WeaponInfoData.BaseClipCapacity = WeaponRow->BaseClipCapacity;
		WeaponInfoData.MaxAmmoCapacity = WeaponRow->MaxAmmoCapacity;
		WeaponInfoData.TimeBetweenShots = WeaponRow->TimeBetweenShots;
		WeaponInfoData.Desc = WeaponRow->Desc;
		//WeaponInfoData.ItemClass = WeaponRow->ItemClass;
		WeaponInfoData.ItemId = WeaponRow->ItemId;
		WeaponInfoData.ItemIconAsset = WeaponRow->ItemIconAsset;
		//WeaponInfoData.ItemTextureAsset = WeaponRow->ItemTextureAsset->ClassDefaultObject
		InWeaponInfos.Add(WeaponInfoData);
	}
}
