// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSItems/INSPickups/INSItems_Pickup.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSPickups/INSPickupBase.h"
#include "INSPickup_Weapon.generated.h"

class AINSWeaponBase;
class AINSPlayerController;
class AINSItems_Pickup;
class USkeletalMeshComponent;
class USkeletalMesh;


/**
 *  weapon pickup
 */
UCLASS(Blueprintable)
class INSURGENCY_API AINSPickup_Weapon : public AINSPickupBase
{
	GENERATED_UCLASS_BODY()

protected:

	/** actual weapon class to spawn when player picks this up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "PickupClass")
		TSubclassOf<AINSWeaponBase> ActualWeaponClass;

	/** how many ammo stored in current clip ,the current clip ammo should stay the same after picked up*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
		int32 CurrentClipAmmo;

	/** how many ammo left should be store in this weapon pick up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
		int32 AmmoLeft;

	

protected:
	//~ begin AActor interface
	virtual void BeginPlay()override;
	virtual void PostInitializeComponents()override;
	virtual void Tick(float DeltaTime)override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	//~ end AActor interface


public:

	/**
	 * set the weapon pick up class
	 * @params WeaponClass the actual weapon class to spawn
	 */
	virtual void SetActualWeaponClass(UClass* WeaponClass) { ActualWeaponClass = WeaponClass; }

	/** get the pick up weapon class */
	virtual UClass* GetActualWeaponClass()const { return ActualWeaponClass; }

	/** give this to a claimed player */
	virtual void GiveTo(class AController* PlayerToGive)override;

	/** set ammo amount that could be looted if player hold the ammo compatible weapon*/
	virtual void SetLootableAmmo(int32 AmmoSize) { CurrentClipAmmo = AmmoSize; }

	/** return current clip ammo this weapon pick up should have stored */
	inline virtual int32 GetLootableAmmoAmount()const { return CurrentClipAmmo; }

	/** set ammo left this weapon should have stored */
	virtual void SetAmmoLeft(int32 AmmoLeftSize) { AmmoLeft = AmmoLeftSize; }

	/** return ammo left this weapon pick up should have stored */
	inline virtual int32 GetAmmoLeft()const { return AmmoLeft; }
};
