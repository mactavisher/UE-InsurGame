// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSItems/INSItems.h"
#include "GameFramework/Actor.h"
#include "INSItems_Pickup.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;

/**
 * general item type for pick ups
 * such as weapon pickups,or resources pick ups
 */
UCLASS(Abstract, NotBlueprintType)
class INSURGENCY_API AINSItems_Pickup : public AINSItems
{
	GENERATED_UCLASS_BODY()
	/** interaction comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "InteractComp", meta = (AllowPrivateAccess = "true"))
	USphereComponent* InteractionComp;

	/** indicate this pick up will be auto picked up by player who get close enough to it */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Config")
	uint8 bAutoPickup:1;
protected:
	/** handle when this pick up is overlapped with characters */
	UFUNCTION()
	virtual void HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** handles when when player leave this pick up */
	UFUNCTION()
	virtual void HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** give this to a claimed player */
	virtual void GiveThisToPlayer(class AController* NewClaimedPlayer);

	virtual void BeginPlay() override;
};
