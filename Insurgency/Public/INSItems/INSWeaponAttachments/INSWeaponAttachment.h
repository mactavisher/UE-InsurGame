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
UCLASS(Abstract, Blueprintable)
class INSURGENCY_API AINSWeaponAttachment : public AINSItems
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh1PComp", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh1p;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh3PComp", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh3p;

	/** weapon that own this attachment */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponOwner")
		AINSWeaponBase* WeaponOwner;

	/** in which slot can this attachment attach to, e.g. underBarrel,muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		TArray<EWeaponAttachmentType> CompatibleSlots;


protected:
	virtual void BeginPlay() override;

	virtual void OnRep_Owner()override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;


public:

	virtual void Tick(float DeltaTime) override;

	virtual void ReceiveAttachmentEquipped(class AINSWeaponBase* WeaponEuippedBy);

	/** return the weapon that own this attachment */
	FORCEINLINE virtual class AINSWeaponBase* GetWeaponOwner()const { return WeaponOwner; }

	/** return the weapon that own this attachment */
	virtual void SetWeaponOwner(class AINSWeaponBase* NewWeaponOwner) { this->WeaponOwner = NewWeaponOwner; }

};
