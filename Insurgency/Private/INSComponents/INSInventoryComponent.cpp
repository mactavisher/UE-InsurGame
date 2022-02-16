// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSInventoryComponent.h"

#include "IDetailTreeNode.h"
#include "INSCharacter/INSCharacter.h"
#include "INSCore/INSGameInstance.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSCore/INSItemManager.h"
#include "INSItems/INSItems.h"

DEFINE_LOG_CATEGORY(LogINSInventory);

UINSInventoryComponent::UINSInventoryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	ItemManager = nullptr;
	OwnerChar =nullptr;
	bInitialized =false;
}

void UINSInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	const UINSGameInstance* const CurrentGameInstance = GetWorld()->GetGameInstance<UINSGameInstance>();
	ItemManager = CurrentGameInstance->GetItemManager();
	InitInventoryItemData();
	SortByWeaponPriority();
	OwnerChar = GetOwner<AINSCharacter>();
	bInitialized = true;
}


void UINSInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UINSInventoryComponent::SortByWeaponPriority()
{
	const uint8 InventorySlotNum = InventorySlots.Num();
	if (InventorySlotNum <= 0)
	{
		return;
	}
	for (uint8 i = 0; i < InventorySlotNum - 1; i++)
	{
		for (uint8 j = 0; j < InventorySlotNum - 1 - i; j++)
		{
			if (InventorySlots[j].Priority > InventorySlots[j + 1].Priority)
			{
				const FInventorySlot& Temp = InventorySlots[j + 1];
				InventorySlots[j + 1] = InventorySlots[j];
				InventorySlots[j] = Temp;
			}
		}
	}
}

void UINSInventoryComponent::InitInventoryItemData()
{
	if (ItemManager)
	{
		for (FInventorySlot& InventorySlot : InventorySlots)
		{
			const int32 ItemId = InventorySlot.ItemId;
			FWeaponInfoData WeaponInfoData;
			ItemManager->GetItemById(ItemId, WeaponInfoData);
			InventorySlot.count = 1;
			InventorySlot.bEquipable = true;
			InventorySlot.Priority = WeaponInfoData.Priority;
			InventorySlot.AmmoLeft = WeaponInfoData.BaseClipCapacity * WeaponInfoData.BaseClipSize;
			InventorySlot.ClipAmmo = WeaponInfoData.BaseClipCapacity;
		}
	}
}

bool UINSInventoryComponent::CheckSlotValid(const uint8 SlotIndex)
{
	if (SlotIndex <= 0 || SlotIndex > 255)
	{
		UE_LOG(LogINSInventory, Warning, TEXT("the given slot index%d is not valid"), SlotIndex);
		return false;
	}
	return true;
}

void UINSInventoryComponent::NotifyInventoryInitialized()
{
	if (OwnerChar)
	{
		OwnerChar->ReceiveInventoryInitialized();
	}
}

void UINSInventoryComponent::InitItemData(AINSItems* InItem)
{
	if (!InItem)
	{
		return;
	}
	FInventorySlot* InventorySlot = GetItemSlot(InItem->GetItemId());
	if (!InventorySlot)
	{
		return;
	}
	const EItemType ItemType = InItem->GetItemType();
	if (ItemType == EItemType::WEAPON)
	{
		AINSWeaponBase* InWeapon = Cast<AINSWeaponBase>(InItem);
		if (InWeapon)
		{
			InWeapon->SetAmmoLeft(InventorySlot->AmmoLeft);
			InWeapon->SetCurrentClipAmmo(InventorySlot->ClipAmmo);
		}
	}
}

void UINSInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FInventorySlot* UINSInventoryComponent::GetItemSlot(uint8 TargetSlotIndex)
{
	for (uint8 i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].SlotId == TargetSlotIndex)
		{
			return (FInventorySlot*)&InventorySlots[i];
		}
	}
	return nullptr;
}

bool UINSInventoryComponent::PutItemInSlot(class AINSItems* Item)
{
	const uint8 TargetSlot = Item->GetInventorySlotIndex();
	FInventorySlot* const Slot = GetItemSlot(TargetSlot);
	if (Item->GetClass()->IsChildOf(AINSWeaponBase::StaticClass()))
	{
		const AINSWeaponBase* const Weapon = Cast<AINSWeaponBase>(Item);
		Slot->AmmoLeft = Weapon->GetAmmoLeft();
		Slot->ClipAmmo = Weapon->GetCurrentClipAmmo();
		Slot->count = 1;
		Slot->bEquipable = true;
		Slot->Priority = Weapon->GetWeaponPriority();
	}
	Slot->ItemId = Item->GetItemId();
	Slot->SkinId = Item->GetSkinId();
	Slot->count = 1;
	return true;
}

uint8 UINSInventoryComponent::GiveBestWeapon()
{
	for (uint8 i = 0; i < InventorySlots.Num(); i++)
	{
		const FInventorySlot CurrentSlot = InventorySlots[i];
		if (CurrentSlot.ItemId > 0 && CurrentSlot.bEquipable)
		{
			return CurrentSlot.SlotId;
		}
	}
	return -1;
}

AINSItems* UINSInventoryComponent::GetItemFromInventory(const int32 ItemId, const uint8 InventorySlotId)
{
	if (!ItemManager)
	{
		return nullptr;
	}
	FInventorySlot* InventorySlot = GetItemSlot(InventorySlotId);
	const uint8 TargetSlotId = InventorySlot == nullptr ? 255 : InventorySlot->SlotId;
	const int32 TargetItemId = InventorySlot == nullptr ? ItemId : InventorySlot->ItemId;
	if (TargetItemId > 0 || TargetSlotId > 0)
	{
		if (InventorySlot->ItemType == EItemType::WEAPON)
		{
			AINSWeaponBase* WeaponItem = ItemManager->CreateWeaponItemInstance(TargetItemId, OwnerChar->GetActorTransform(), OwnerChar->GetOwner(), OwnerChar, TargetSlotId);
			return WeaponItem;
		}
	}
	return nullptr;
}

UClass* UINSInventoryComponent::FindItemClassById(int32 ItemId)
{
	if (ItemId < 0)
	{
		return nullptr;
	}
	return nullptr;
}

void UINSInventoryComponent::SetItemManager(UINSItemManager* InItemManger)
{
	this->ItemManager = InItemManger;
}
