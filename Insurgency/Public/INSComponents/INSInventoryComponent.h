// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INSInventoryComponent.generated.h"

class UTexture2D;
class AINSWeaponBase;
class AINSWeaponAttachment;

USTRUCT(BlueprintType)
struct FInvetorySlot
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
	int32 Inventory;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INSURGENCY_API UINSInventoryComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	TArray<FInvetorySlot> InventorySlots;

protected:
	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void InitializeComponent()override;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent interface

public:
	virtual FInvetorySlot* GetItemSlot(uint8 TargetSlotIndex);
	virtual bool PutItemInSlot(class AINSWeaponBase* Item);
	
};
