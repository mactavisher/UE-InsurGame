// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Insurgency/Insurgency.h"
#include "INSPickupBase.generated.h"

class USphereComponent;
class UBoxComponent;

USTRUCT(BlueprintType)
struct FRepPickupInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString VisualAssetPath;

	UPROPERTY()
	uint8 skinIndex;

	FRepPickupInfo()
		: VisualAssetPath("None")
		  , skinIndex(0)
	{
	}
};


INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSPickup, Log, All);

UCLASS()
class INSURGENCY_API AINSPickupBase : public AActor
{
	GENERATED_UCLASS_BODY()
protected:
	/** indicates if this pickup is active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_bIsActive, Category = "InventoryItems|Status")
	uint8 bIsActive : 1;

	/** indicates the item type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "InventoryItem|ItemType")
	EItemType ItemType;

	/** texture to draw when player enters this pickup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IventoryItem|UI")
	UTexture2D* ItemIconTexture;

	/** indicates how much time needed that the player claimed this pickup for them */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InventoryItems|Interact")
	float InteractTime;

	/** info to  draw when player enters this pickup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemName")
	FText ItemDisplayName;

	/** sphere shaped interaction comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "InteractComp", meta = (AllowPrivateAccess = "true"))
	USphereComponent* InteractionComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="VisualMesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* VisualMeshComp;

	/** Box component providing simple physics for pickups if enabled  */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "InteractComp", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* SimpleCollisionComp;

	/** indicate this pick up will be auto picked up by player who get overlapped with this  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	uint8 bAutoPickup : 1;

	/** indicate this pick up will be auto picked up by player who get overlapped with this  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	uint8 bAutoDestroy : 1;

	UPROPERTY(ReplicatedUsing = "OnRep_PickupInfo")
	FRepPickupInfo RepPickupInfo;

	UFUNCTION()
	virtual void OnRep_PickupInfo();

protected:
	// ~begin AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void GatherCurrentMovement() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/**
	 * @Desc set the exclusive owner of this pick up
	 * @Param  NewOwner Exclusive owner to set for this pick up
	 */
	virtual void SetOwner(AActor* NewOwner) override;
	// ~end AActor interface

protected:
	/** active state rep notify */
	UFUNCTION()
	virtual void OnRep_bIsActive();

	/** owner rep notify,override */
	virtual void OnRep_Owner() override;


	/**
	 * @Desc handles overlap with characters
	 * @Param OverlappedComponent component that overlaps with other
	 * @Param OtherComp other component that overlaps with this pickup
	 * @Param OtherActor OtherActor that overlaps with this pickup
	 * @Param OtherBodyIndex OtherBodyIndex
	 * @Param bFromSweep bFromSweep
	 * @Param SweepResult SweepResult
	 */
	UFUNCTION()
	virtual void HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * @Desc handles end overlap with characters
	 * @Param OverlappedComponent component that overlaps with other
	 * @Param OtherComp other component that overlaps with this pickup
	 * @Param OtherActor OtherActor that overlaps with this pickup
	 * @Param OtherBodyIndex OtherBodyIndex
	 */
	UFUNCTION()
	virtual void HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	/** enable actor primary tick */
	virtual void EnableTick();

	/** disable actor primary tick */
	virtual void DisableTick();

public:
	/**
	 * the owner we set will typically be controller,but it's may be an AIController,
	 * so we provide a template here,return value might be null if class Type not compatible
	 */
	template <typename T>
	T* GetOwnerPlayer() const
	{
		return Cast<T>(GetOwner());
	}

	/**
	 * @Desc receive from a player if the interact time finished
	 * @Param Player Player who finishes it's interaction time
	 */
	virtual void ReceiveInteractFinished(class AController* Player);

	/**
	 * @Desc Set the item type for this pick up
	 * @Param newType new item type to set for this pickup
	 */
	virtual void SetItemType(EItemType NewType) { this->ItemType = NewType; }

	/**
	 * @Desc give this pick up to a player
	 * @Param PlayerToGive Player to give this pick up to
	 */
	virtual void GiveTo(class AController* PlayerToGive);

	/** returns the Item Icon Texture*/
	virtual UTexture2D* GetItemIcon() const { return ItemIconTexture; }

	/** returns the item type */
	virtual EItemType GetItemType() const { return ItemType; }

	/** return the interact time need for this pick up */
	virtual float GetInteractTime() const { return InteractTime; }

	/** return the display info for this pick up */
	virtual FText GetItemDisplayName() const { return ItemDisplayName; }

	virtual void SetRepPickupInfo(FRepPickupInfo InRepInfo) { RepPickupInfo = InRepInfo; }

	FORCEINLINE UBoxComponent* GetSimpleCollisionComp() const { return SimpleCollisionComp; }

	FORCEINLINE USphereComponent* GetInteractComp() const { return InteractionComp; }

	FORCEINLINE UStaticMeshComponent* GetVisualMeshComp() const { return VisualMeshComp; };
};
