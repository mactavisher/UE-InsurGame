// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "INSWeaponAttachment.generated.h"

class AINSWeaponBase;
class USkeletalMeshComponent;

UCLASS()
class INSURGENCY_API AINSWeaponAttachment : public AActor
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh1PComp", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh1p;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh1PComp", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh3p;

	/** weapon that own this attachment */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_OwnerWeapon, Category = "WeaponOwner")
	 AINSWeaponBase* WeaponOwner;


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnRep_OwnerWeapon();

	virtual class AController* GetOwnerPlayer();

	virtual class AController* GetOwingPlayer();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;


public:

	virtual void Tick(float DeltaTime) override;

	/** return the weapon that own this attachment */
	FORCEINLINE virtual class AINSWeaponBase* GetWeaponOwner()const { return WeaponOwner; }

	/** return the weapon that own this attachment */
	virtual void SetWeaponOwner(class AINSWeaponBase* NewWeaponOwner) { this->WeaponOwner = NewWeaponOwner; }

};
