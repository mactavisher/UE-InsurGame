// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "INSWeaponAnimInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UINSWeaponAnimInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INSURGENCY_API IINSWeaponAnimInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/** ~~--------------------------------------------------------------
       delegates declare ----------------------------------------------*/

	/** fire delegate */
	FScriptDelegate FireDelegate;

	/** reload delegate */
	FScriptDelegate StartReloadDelegate;

	/** switch fire mode delegate */
	FScriptDelegate StartSwitchFireModeDelegate;

	/** equip weapon delegate */
	FScriptDelegate StartEquipDelegate;

	/** idle weapon delegate */
	FScriptDelegate StartWeaponIdleDelegate;

	/** equip weapon delegate */
	FScriptDelegate StartAimDelegate;

	/** idle weapon delegate */
	FScriptDelegate StopAimDelegate;

	/** equip weapon delegate */
	FScriptDelegate StartSprintDelegate;

	/** idle weapon delegate */
	FScriptDelegate StopSprintDelegate;


public:

	/** ~~--------------------------------------------------------------
	   declare interface Functions ------------------------------------*/

	/** fire interface function*/
	UFUNCTION()
	virtual void PlayFireAnim() {};

	/** switch fire mode interface function */
	UFUNCTION()
	virtual float PlaySwitchFireModeAnim() { return 0.f; };

	/** Reload interface function */
	UFUNCTION()
	virtual float PlayReloadAnim( bool bIsDry) {return 0.f;};

	/** equip weapon interface function */
	UFUNCTION()
	virtual float PlayWeaponStartEquipAnim() { return 0.f; };

	/** base weapon pose interface function */
	UFUNCTION()
	virtual void PlayWeaponBasePose() {};

	/** base weapon pose interface function */
	UFUNCTION()
	virtual void PlayWeaponIdleAnim() {};

	UFUNCTION()
	virtual void PlayAimAnim() {};

	UFUNCTION()
	virtual void PlayStopAimAnim() {};

	UFUNCTION()
		virtual void PlaySprintAnim() {};

	UFUNCTION()
		virtual void OnWeaponAnimDelegateBindingFinished() {}

	UFUNCTION()
		virtual void StopPlaySprintAnim() {};

	/** bind interface functions to events */
	virtual void BindWeaponAnimDelegate() {};

	/** unbind interface functions from events */
	virtual void UnbindWeaponAnimDelegate() {};

	virtual void SetWeaponBasePoseType(EWeaponBasePoseType NewWeaponBasePoseType) {}
};
