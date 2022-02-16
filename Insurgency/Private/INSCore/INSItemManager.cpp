// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCore/INSItemManager.h"

#include "IDetailTreeNode.h"
#include "INSCharacter/INSCharacter.h"
#include "INSCore/INSGameInstance.h"
#include "INSItems/INSItems.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogINSItemManager);

UINSItemManager::UINSItemManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ItemDataTable = nullptr;
	WeaponDataTable = nullptr;
	OwningGameInstance = nullptr;
}

void UINSItemManager::SetItemTable(UDataTable* NewItemDataTable)
{
	this->ItemDataTable = NewItemDataTable;
}

void UINSItemManager::SetWeaponTable(UDataTable* NewWeaponTable)
{
	WeaponDataTable = NewWeaponTable;
}


void UINSItemManager::SetOwingGameInstance(UINSGameInstance* NewGameInstance)
{
	this->OwningGameInstance = NewGameInstance;
}

void UINSItemManager::GetWeaponItemById(int32 ItemId, FWeaponInfoData& OutItemInfo)
{
	TArray<FWeaponTableRow*> AllRows;
	WeaponDataTable->GetAllRows(TEXT(""), AllRows);
	FWeaponTableRow* MatchedRow = nullptr;
	for (FWeaponTableRow* ItemDataRow : AllRows)
	{
		if (ItemDataRow->ItemId == ItemId)
		{
			MatchedRow = ItemDataRow;
			break;
		}
	}
	if (MatchedRow)
	{
		OutItemInfo.CopyDataFromTable(MatchedRow);
	}
	else
	{
		UE_LOG(LogINSItemManager, Warning, TEXT("Item of ItemId:%d not found"), ItemId);
	}
}

void UINSItemManager::GetItemById(int32 ItemId, FItemInfoData& OutItemInfo)
{
	TArray<FItemTableRow*> AllRows;
	ItemDataTable->GetAllRows(TEXT(""), AllRows);
	FItemTableRow* MatchedRow = nullptr;
	for (FItemTableRow* const ItemDataRow : AllRows)
	{
		if (ItemDataRow->ItemId == ItemId)
		{
			MatchedRow = ItemDataRow;
			break;
		}
	}
	if (MatchedRow)
	{
		OutItemInfo.CopyDataFromTable(MatchedRow);
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

AINSItems* UINSItemManager::CreateItemInstance(int32 ItemId, const FTransform& InitialTransform, AActor* Owner, APawn* Instigator)
{
	if (ItemId > 0)
	{
		FItemInfoData ItemInfo;
		GetItemById(ItemId, ItemInfo);
		if (ItemInfo.ItemClass)
		{
			UClass* ItemClass = ItemInfo.ItemClass.Get();
			AINSItems* Item = GetWorld()->SpawnActorDeferred<AINSItems>(ItemClass, InitialTransform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (Item)
			{
				UGameplayStatics::FinishSpawningActor(Item, InitialTransform);
			}
			Item->SetItemInfo(ItemInfo);
			return Item;
		}
	}
	return nullptr;
}

AINSWeaponBase* UINSItemManager::CreateWeaponItemInstance(int32 ItemId, const FTransform& InitialTransform, AActor* Owner, APawn* Instigator,const uint8 InventorySlotIndex)
{
	if (ItemId > 0)
	{
		FWeaponInfoData WeaponInfo;
		GetWeaponItemById(ItemId, WeaponInfo);
		const FSoftObjectPath SoftObjectPath = WeaponInfo.ItemClass.ToSoftObjectPath();
		UClass* const ItemClass = LoadClass<AINSWeaponBase>(this, *SoftObjectPath.ToString());
		AINSWeaponBase* Item = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(ItemClass, InitialTransform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Item)
		{
			Item->SetItemInfo(WeaponInfo);
			Item->SetWeaponInfoData(WeaponInfo);
			Item->SetInventorySlotIndex(InventorySlotIndex);
			Item->SetAutonomousProxy(true);
			Item->SetItemId(ItemId);
			Item->SetWeaponState(EWeaponState::NONE);
			Item->SetOwnerCharacter(Cast<AINSCharacter>(Instigator));
			Item->SetOwner(Instigator->GetController());
			UGameplayStatics::FinishSpawningActor(Item, InitialTransform);
		}
		return Item;
	}
	return nullptr;
}


void UINSItemManager::GetAllItemInfos(TArray<FItemInfoData> InItemInfos)
{
}

void UINSItemManager::GetAllWeaponsInfos(TArray<FWeaponInfoData>& InWeaponInfos)
{
	TArray<FWeaponTableRow*> AllRows;
	ItemDataTable->GetAllRows(TEXT(""), AllRows);
	if (AllRows.Num() <= 0)
	{
		return;
	}
	for (FWeaponTableRow* const WeaponRow : AllRows)
	{
		FWeaponInfoData WeaponInfoData;
		WeaponInfoData.CopyDataFromTable(WeaponRow);
		InWeaponInfos.Add(WeaponInfoData);
	}
}
