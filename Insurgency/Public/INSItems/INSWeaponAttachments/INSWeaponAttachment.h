// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Insurgency/Public/INSItems/INSItems.h"
#include "Insurgency/Insurgency.h"
#include "INSWeaponAttachment.generated.h"

class AINSWeaponBase;
class USkeletalMeshComponent;

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
	USkeletalMeshComponent* AttachmentMesh;

	/** weapon that own this attachment */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_OwnerWeapon, Category = "WeaponOwner")
	AINSWeaponBase* WeaponOwner;

	/** in which slot can this attachment attach to, e.g. underBarrel,muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	TArray<EWeaponAttachmentType> CompatibleWeaponSlots;

	/** in which slot can this attachment attach to, e.g. underBarrel,muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	EWeaponAttachmentType CurrentAttachmentType;

	UPROPERTY()
	AINSWeaponAttachment* ClientVisualAttachment;

	UPROPERTY()
	uint8 bClientVisualAttachment:1;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="WeaponAttachments")
	uint8 AttachedSlotIndex;


protected:
	UPROPERTY()
	uint8 bChangeWeaponBasePoseType:1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="WeaponAttachments")
	EWeaponBasePoseType TargetWeaponBasePoseType;


protected:
	/** attach the attachment to weapon attachment slot */
	virtual void AttachToWeaponSlot();

	/** create attachment for client visual only */
	virtual void CreateClientVisualAttachment();

	UFUNCTION()
	virtual void OnRep_OwnerWeapon();

	virtual void UpdateWeaponBasePoseType();

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
	virtual void SetWeaponOwner(class AINSWeaponBase* NewWeaponOwner) { this->WeaponOwner = NewWeaponOwner; }

    /**get the client fake attachment instance */
	FORCEINLINE virtual AINSWeaponAttachment* GetClientVisualAttachment() const { return ClientVisualAttachment; }

    /** get the attachmentmesh comp*/
	FORCEINLINE virtual class USkeletalMeshComponent* GetAttachmentMeshComp() const;

	virtual void SetClientVisualAttachment(class AINSWeaponAttachment* NewClientAttachment) { ClientVisualAttachment = NewClientAttachment; }

	inline virtual EWeaponBasePoseType GetTargetWeaponBasePoseType() const { return TargetWeaponBasePoseType; }

	virtual void SetTargetWeaponBasePoseType(const EWeaponBasePoseType NewTargetWeaponBasePoseType) { this->TargetWeaponBasePoseType = TargetWeaponBasePoseType; }

	virtual EWeaponAttachmentType GetCurrentAttachmentType() const
	{
		return CurrentAttachmentType;
	}

	virtual void SetCurrentAttachmentType(const EWeaponAttachmentType NewCurrentAttachmentType)
	{
		this->CurrentAttachmentType = NewCurrentAttachmentType;
	}
};
