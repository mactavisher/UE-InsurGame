// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INSCore/INSItemManager.h"
#include "INSInventoryComponent.generated.h"

class UTexture2D;
class AINSWeaponBase;
class AINSWeaponAttachment;
class UINSItemManager;
class AINSItems;
class AINSCharacter;
INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSInventory, Log, All);
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_USTRUCT_BODY()
	/** slot id */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 SlotId;

	/** item id  stores in this inventory*/
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	int32 ItemId;

	/** store the weapon class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<TSubclassOf<AINSWeaponAttachment>> WeaponAttachmentClass;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	TArray<int32> WeaponAttachmentItemIds;

	/** Skin for weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 SkinId;

	/** cache the current clip ammo */
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly)
	int32 ClipAmmo;

	/** cache the ammo left */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	int32 AmmoLeft;

	/** how many pieces of this item do we have */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	int32 count;

	/** how many pieces of this item do we have */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	uint8 Priority;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	EItemType ItemType;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	uint8 bEquipable:1;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INSURGENCY_API UINSInventoryComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> InventorySlots;
	
	UPROPERTY()
	UINSItemManager* ItemManager;

	UPROPERTY()
	AINSCharacter* OwnerChar;
	
	UPROPERTY()
	uint8 bInitialized:1;


protected:
	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void InitializeComponent()override;
	virtual void SortByWeaponPriority();
	virtual void InitInventoryItemData();
	virtual bool CheckSlotValid(const uint8 SlotIndex);
	virtual void NotifyInventoryInitialized();
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent interface

public:
	virtual FInventorySlot* GetItemSlot(uint8 TargetSlotIndex);
	virtual bool PutItemInSlot(class AINSItems* Item);
	virtual uint8 GiveBestWeapon();
	virtual  AINSItems* GetItemFromInventory(const int32 ItemId,const uint8 InventorySlotId);
	virtual UClass* FindItemClassById(int32 ItemId);
	virtual UINSItemManager* GetItemManager()const{return ItemManager;}
	virtual void SetItemManager(UINSItemManager* InItemManger);
	virtual void InitItemData(class AINSItems* InItem);
	virtual bool GetInitialized()const{return bInitialized;}
};
