// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Insurgency/Public/INSItems/INSItems.h"
#include "Insurgency/Insurgency.h"
#include "INSWeaponAttachment.generated.h"

class AINSWeaponBase;
class UStaticMeshComponent;

/**
 * weapon attachment for weapon to equip
 * such as grip,scope etc.
 * each weapon attachment will modify weapon's properties or behavior
 */
UCLASS(Abstract, NotBlueprintable)
class INSURGENCY_API AINSWeaponAttachment : public AINSItems
{
	GENERATED_UCLASS_BODY()
protected:
	/** attachment visual skeletal component , should have no collision(Physics) enabled */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AttachmentMeshComp", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* AttachmentMeshComp;

	/** weapon that own this attachment */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_OwnerWeapon, Category = "WeaponOwner")
		AINSWeaponBase* WeaponOwner;

	/** in which slot can this attachment attach to, e.g. underBarrel,muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		TArray<EWeaponAttachmentType> CompatibleWeaponSlots;

	/** in which slot can this attachment attach to, e.g. underBarrel,muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		EWeaponAttachmentType AttachmentType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponAttachments")
		uint8 AttachedSlotIndex;

	/** Target FOV value,for optics only */
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="FOV")
	   float TargetFOV;


protected:
	/** for grips typically, such as fore grips */
	UPROPERTY()
		uint8 bChangeWeaponBasePoseType : 1;

	/** for bolt rifles typically,indicates if this attachment will block weapon quick reloading */
	UPROPERTY()
		uint8 bBlockQuickBoltRifileReloading : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponAttachments")
		EWeaponBasePoseType TargetWeaponBasePoseType;


protected:
	/** attach the attachment to weapon attachment slot */
	virtual void AttachToWeaponSlot();

	UFUNCTION()
		virtual void OnRep_OwnerWeapon();

	virtual void CheckAndUpdateWeaponBasePoseType();

protected:
	// ~ Begin AActor interface
	virtual void Tick(float DeltaTime) override;
	virtual void OnRep_Owner() override;
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ End AActor interface

public:
	/** called when this attachment has been attached to the weapon */
	virtual void ReceiveAttachmentEquipped(class AINSWeaponBase* WeaponEquippedBy);

	/** return the weapon that own this attachment */
	FORCEINLINE virtual class AINSWeaponBase* GetWeaponOwner() const { return WeaponOwner; }

	/** set the weapon that own this attachment */
	virtual void SetWeaponOwner(class AINSWeaponBase* NewWeaponOwner);


	/** get the attachment mesh comp*/
	FORCEINLINE virtual class UStaticMeshComponent* GetAttachmentMeshComp() const;

	/** gets the target FOV of this attachment ,for optics only */
	virtual float GetTargetFOV()const { return TargetFOV; }

	inline virtual EWeaponBasePoseType GetTargetWeaponBasePoseType() const { return TargetWeaponBasePoseType; }

	virtual FVector GetBarrelLocation() { return FVector::ZeroVector; }

	virtual void SetTargetWeaponBasePoseType(const EWeaponBasePoseType NewTargetWeaponBasePoseType) { this->TargetWeaponBasePoseType = TargetWeaponBasePoseType; }

	virtual EWeaponAttachmentType GetCurrentAttachmentType() const
	{
		return AttachmentType;
	}

	virtual void SetCurrentAttachmentType(const EWeaponAttachmentType NewCurrentAttachmentType)
	{
		this->AttachmentType = NewCurrentAttachmentType;
	}
};
