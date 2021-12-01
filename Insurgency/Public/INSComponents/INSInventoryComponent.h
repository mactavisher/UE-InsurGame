// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INSCore/INSItemManager.h"
#include "INSItems/INSItems.h"
#include "INSInventoryComponent.generated.h"

class UTexture2D;
class AINSWeaponBase;
class AINSWeaponAttachment;
class UINSItemManager;

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_USTRUCT_BODY()
		/** texture represent this item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite);
	UTexture2D* ItemTexture;

	/** icon represent this item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite);
	UTexture2D* ItemIcon;

	/** slot id */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite);
	uint8 SlotId;

	/** store the weapon class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite);
	TSubclassOf<AINSWeaponBase> SlotWeaponClass;

	/** store the weapon class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite);
	TArray<TSubclassOf<AINSWeaponAttachment>> WeaponAttachmentClass;

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

protected:
	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void InitializeComponent()override;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent interface

public:
	virtual FInventorySlot* GetItemSlot(uint8 TargetSlotIndex);
	virtual bool PutItemInSlot(class AINSWeaponBase* Item);
	virtual UClass* GiveBestWeapon(uint8 &OutSlotIndex);
	virtual UINSItemManager* GetItemManager()const{return ItemManager;}

	virtual void SetItemManager(UINSItemManager* InItemManger);
	
};
