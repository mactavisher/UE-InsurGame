// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "INSProjectileMovementComponent.generated.h"

class AINSProjectileBase;
/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY()
	FVector SpawnLocation;

	UPROPERTY()
	uint8 bScanTraceProjectile : 1;

	UPROPERTY()
	uint8 bScanTraceCondSet : 1;

	UPROPERTY()
	AINSProjectileBase* OwnerProjectile;

	UPROPERTY()
	float ScanTraceMoveTime;

	UPROPERTY()
	float CurrentScanTraceMoveTime;

protected:
	//~Begin UProjectileMovementComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	//~End UProjectileMovementComponent Interface

public:
	virtual void SetScanTraceProjectile(bool bFromScanTrace);
	virtual void SetOwnerProjectile(AINSProjectileBase* NewProjectile) { OwnerProjectile = NewProjectile; }
	virtual void EnableTick(const bool bEnableTick);
	virtual void SetScanTraceMoveTime(const float ScanTraceTime);
	FORCEINLINE virtual AINSProjectileBase* GetOwnerProjectile() const { return OwnerProjectile; }
	FORCEINLINE virtual bool GetIsScanTraceProjectile() const { return bScanTraceProjectile; }
};
