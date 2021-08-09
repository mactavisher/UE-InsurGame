// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSInventoryComponent.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

UINSInventoryComponent::UINSInventoryComponent(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void UINSInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UINSInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UINSInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FInvetorySlot* UINSInventoryComponent::GetItemSlot(uint8 TargetSlotIndex)
{
	for (uint8 i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].SlotId == TargetSlotIndex)
		{
			return (FInvetorySlot*)&InventorySlots[i];
		}
	}
	return nullptr;
}

bool UINSInventoryComponent::PutItemInSlot(class AINSWeaponBase* Item)
{
	const uint8 TargetSlot = Item->GetInventorySlotIndex();
	FInvetorySlot* const Slot = GetItemSlot(TargetSlot);
	Slot->AmmoLeft = Item->AmmoLeft;
	Slot->ClipAmmo = Item->CurrentClipAmmo;
	Slot->SlotWeaponClass = Item->GetClass();
	Slot->count = 1;
	return true;
}

