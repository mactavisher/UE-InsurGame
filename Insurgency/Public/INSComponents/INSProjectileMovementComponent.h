// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "INSProjectileMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category = "Projectile")
	float ScanHitTime;

	UPROPERTY()
	FVector SpawnLocation;

	UPROPERTY()
	uint8 bScanTraceProjectile : 1;

	UPROPERTY()
	TObjectPtr<AINSProjectile> OwnerProjectile;

protected:
	//~Begin UProjectileMovementComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;
	virtual void BeginPlay()override;
	//~End UProjectileMovementComponent Interface

public:
	virtual void SetScanTraceProjectile(bool bFromScanTrace) { bScanTraceProjectile = bFromScanTrace; }

	virtual void SetOwnerProjectile(AINSProjectile* NewProjectile) { OwnerProjectile = NewProjectile; }

	virtual class AINSProjectile* GetOWnerProjectile()const { return OwnerProjectile; }
};
