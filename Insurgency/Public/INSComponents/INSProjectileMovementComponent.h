// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "INSProjectileMovementComponent.generated.h"

class AINSProjectile;
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
	AINSProjectile* OwnerProjectile;

protected:
	//~Begin UProjectileMovementComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;
	virtual void BeginPlay()override;
	//~End UProjectileMovementComponent Interface

public:
	virtual void SetScanTraceProjectile(bool bFromScanTrace) { bScanTraceProjectile = bFromScanTrace; }

	virtual void SetOwnerProjectile(AINSProjectile* NewProjectile) { OwnerProjectile = NewProjectile; }

	FORCEINLINE virtual class AINSProjectile* GetOwnerProjectile()const { return OwnerProjectile; }

	FORCEINLINE virtual bool GetIsScanTraceProjectile()const{return bScanTraceProjectile;}
};
