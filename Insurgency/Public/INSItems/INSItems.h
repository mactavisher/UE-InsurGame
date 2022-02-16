// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "INSAssets/INSWeaponAssets.h"
#include "Insurgency/Insurgency.h"
#include "INSItems.generated.h"

class USphereComponent;
class UTexture2D;
class AINSPlayerController;
class AINSPlayerCharacter;
class UINSInventoryComponent;
struct FInventorySlot;
struct FItemInfoData;
struct FWeaponInfoData;

UCLASS()
class INSURGENCY_API AINSItems : public AActor
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "InventoryItem|ItemType")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Item")
	int32 ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Item")
	int32 SKinId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Item")
	uint8 InventorySlotIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Item")
	struct FItemInfoData ItemInfoData;

	UPROPERTY()
	uint8 bInLobbyMode:1;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual EItemType GetItemType() const { return ItemType; }
	//this is used for child weapon classes to override;
	virtual EWeaponType GetWeaponType() const { return EWeaponType::NONE; }
	virtual void SetItemType(EItemType NewType) { this->ItemType = NewType; }
	virtual void ShowItemIcon(class AController* PlayerInstigator, class ACharacter* DetectPlayerCharacter);
	virtual void EnableTick();
	virtual void DisableTick();
	virtual void OnRep_Owner() override;
	virtual void InitItemInfoByInventorySlot(const FInventorySlot& InventorySlot);
	virtual bool GetIsInLobbyMode() const { return bInLobbyMode; }
	virtual void SetIsInLobbyMode(bool IsLobbyItem) { bInLobbyMode = IsLobbyItem; }
	/** gets the item id*/
	virtual int32 GetItemId() const { return ItemId; }
	virtual void SetItemId(int32 InItemId) { ItemId = InItemId; }
	virtual int32 GetSkinId() const { return SKinId; }
	virtual void SetSkinId(int32 InSkinId) { SKinId = InSkinId; }
	virtual int32 GetInventorySlotIndex() const { return InventorySlotIndex; }
	virtual void SetInventorySlotIndex(uint8 SlotIndex) { InventorySlotIndex = SlotIndex; }
	virtual void SetItemInfo(FItemInfoData& NewItemInfoData);

	/**
	 * return if this item is a weapon
	 */
	virtual bool IsWeapon() const { return ItemType == EItemType::WEAPON; }

public:
	/**
	 * the owner we set will typically be controller,but it's may be an AIController,
	 * so we provide a template here,return value might be null if class Type not compatible
	 */
	template <typename T>
	T* GetOwnerPlayer() const { return Cast<T>(GetOwner()); }
};
