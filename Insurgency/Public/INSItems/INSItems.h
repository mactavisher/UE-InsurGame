// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Insurgency/Insurgency.h"
#include "INSItems.generated.h"

class USphereComponent;
class UTexture2D;
class AINSPlayerController;

UCLASS()
class INSURGENCY_API AINSItems : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InventoryItems|Interact")
		USphereComponent* InteractCollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_bIsActive, Category = "InventoryItems|Status")
		uint8 bIsActive : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "InventoryItems|IntemType")
		uint8 bIsWeapon : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "InventoryItem|ItemType")
		EItemType ItemType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IventoryItem|UI")
		UTexture2D* ItemIconTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InventoryItems|Interact")
		float InteractTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemName")
		FName ItemDisplayName;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void NotifyCharacterEnterIventoryItem(class AINSPlayerCharacter* CharacterToNotify);

	virtual void NotifyCharacterLeaveInventoryItem(class AINSPlayerCharacter* CharacterToNotify);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	UFUNCTION()
		virtual void OnRep_bIsActive();

	UFUNCTION()
		virtual void HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		virtual void HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:

	virtual void ReceiveDetectedBy(class AController* PlayerInstigator, class ACharacter* DetectPlayerCharacter);

	virtual void ReceiveInteractFinished(class AController* Player);

	virtual EItemType GetItemType()const { return ItemType; }

	virtual void SetItemType(EItemType NewType) { this->ItemType = NewType; }

	virtual void ShowItemIcon(class AController* PlayerInstigator, class ACharacter* DetectPlayerCharacter);

	virtual UTexture2D* GetItemDisplayIcon()const { return ItemIconTexture; }

	virtual void SetItemDisplayIcon(UTexture2D* NewItemIcon) { this->ItemIconTexture = ItemIconTexture; }

	virtual void EnableTick();

	virtual void DisableTick();

	virtual float GetInteractTime()const { return InteractTime; }

	virtual FName GetItemDisplayName()const { return ItemDisplayName; }

public:
	/** return the current owner owner player */
	virtual AINSPlayerController* GetOwnerPlayer();
};
