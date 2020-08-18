// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSItems/INSPickups/INSItems_Pickup.h"
#include "INSPickup_Weapon.generated.h"

class AINSWeaponBase;
class AINSPlayerController;
class AINSItems_Pickup;
class USkeletalMeshComponent;

/**
 *  weapon pickup
 */
UCLASS()
class INSURGENCY_API AINSPickup_Weapon : public AINSItems_Pickup
{
	GENERATED_UCLASS_BODY()
protected:

	/** actual weapon class to spawn when player picks this up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "PickupClass")
		TSubclassOf<AINSWeaponBase> ActualWeaponClass;

	/** create a visual mesh for this weapon pick up actor */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"),Category="VisualMesh")
		USkeletalMeshComponent* VisualMeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated,ReplicatedUsing=OnRep_VisualMesh, Category = "PickupClass")
	    USkeletalMesh* VisualMesh;

	/** destroy this pick up if nobody touches it  */
	UPROPERTY()
		FTimerHandle DestoryTimer;

	/** how many ammo stored in current clip */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
		int32 CurrentClipAmmo;

	/** how many ammo left should be store in this weapon pick up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
		int32 AmmoLeft;

	/** if no player interact with this item ,start destroy it */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timer")
		float DestroyTime;

	/** player who want this pick up */
	TWeakObjectPtr<AController> ClaimedPlayer;

public:
	/** set the weapon pick up class  */
	virtual void SetActualWeaponClass(UClass* WeaponClass) { ActualWeaponClass = WeaponClass; }

	/** get the pick up weapon class */
	virtual UClass* GetActualWeaponClass()const { return ActualWeaponClass; }

	/** replication support */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	/** handle when this pick up is overlapped with characters */
	virtual void HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)override;

	/** handles when when player leave this pick up */
	virtual void HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)override;

	/** give this to a claimed player */
	virtual void GiveThisToPlayer(class AController* NewClaimedPlayer);

	/** begin play */
	virtual void BeginPlay()override;

	/** set current clip ammo that this weapon pick up should have stored */
	virtual void SetCurrentClipAmmo(int32 AmmoSize) { CurrentClipAmmo = AmmoSize; }

	/** return current clip ammo this weapon pick up should have stored */
	inline virtual int32 GetCurrentClipAmmo()const{return CurrentClipAmmo;}

	/** set ammo left this weapon should have stored */
	virtual void SetAmmoLeft(int32 AmmoLeftSize) { AmmoLeft = AmmoLeftSize; }

	/** return ammo left this weapon pick up should have stored */
	inline virtual int32 GetAmmoLeft()const { return AmmoLeft; }

	/** set player who claimed for this weapon pick up */
	virtual void SetClaimedPlayer(AController* ClaimedBy) { ClaimedPlayer = ClaimedBy; }

	/** return player who claimed for this weapon pick up */
	inline virtual AController* GetClaimedPlayer();

	/** set the visual mesh of this visual mesh comp */
	virtual void SetViualMesh(USkeletalMesh* NewVisualMesh);

	/** after OnRep_Movent called,sync transformation from server replicated transform info */
	virtual void PostNetReceiveLocationAndRotation()override;

	/** optimized version of ReplicatedMovent info  */
	virtual void GatherCurrentMovement()override;

	/** set the visual skin mesh for the skeletal mesh component */
	virtual void SetSkinMeshComp(USkeletalMeshComponent* NewVisualMeshComp) { VisualMeshComp = NewVisualMeshComp; }

	
	UFUNCTION()
	virtual void OnRep_VisualMesh();

	FORCEINLINE virtual USkinnedMeshComponent* GetVisualMeshComp()const { return VisualMeshComp; }

	/** destroy this weapon pick up when nobody touches it for a period of time */
	UFUNCTION()
	virtual void DestroyThisWeaponPickup();
};
