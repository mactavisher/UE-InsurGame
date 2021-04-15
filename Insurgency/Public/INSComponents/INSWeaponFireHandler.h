// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INSWeaponFireHandler.generated.h"

class AINSWeaponBase;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INSURGENCY_API UINSWeaponFireHandler : public UActorComponent
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

	/** fire interval between each shot */
	UPROPERTY()
		float LastFireTime;

	UPROPERTY()
	    EWeaponFireMode CurrentFireMode;

	UPROPERTY()
	FTimerHandle FullAutoFireTimer;

	UPROPERTY()
	FTimerHandle SemiAutoFireTimer;

	UPROPERTY()
    FTimerHandle SemiFireTimer;

protected:
	//~ begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ end UActorComponent Interface

	UFUNCTION()
		virtual void FireShot();

	virtual void ClearFiring();

public:
	virtual void SetOwnerWeapon(class AINSWeaponBase* NewWeapon);

	virtual void BeginWeaponFire(enum EWeaponFireMode NewFireMode);

	virtual void StopWeaponFire();

	virtual bool CheckCanFireAgian();

	virtual void RefireAndCheckTimer();

};
