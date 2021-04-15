// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INSWeaponFireManager.generated.h"

class AINSWeaponBase;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INSURGENCY_API UINSWeaponFireManager : public UActorComponent
{
	GENERATED_UCLASS_BODY()

protected:
	/** the weapon owner of this component */
	UPROPERTY()
	AINSWeaponBase* OwnerWeapon;

	/** the weapon owner of this component */
	UPROPERTY()
	uint8 bIsFiring : 1;

	/** the weapon owner of this component */
	UPROPERTY()
	uint8 bWantsToFire : 1;

	/** CurrentSemiAutofire count */
	UPROPERTY()
	uint8 SemiAutoCount;

	UPROPERTY()
	float ShotTimeRemaining;

	/** fire interval between each shot */
	UPROPERTY()
	float FireInterval;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetOwnerWeapon(class AINSWeaponBase* NewWeapon);

	virtual void BeginFire(EWeaponFireMode CurrentFireMode);

	virtual void StopFire();

	virtual bool CheckCanFireAgian();
};
