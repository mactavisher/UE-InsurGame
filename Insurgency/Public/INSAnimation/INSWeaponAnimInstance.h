// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "INSInterfaces/INSWeaponAnimInterface.h"
#include "INSWeaponAnimInstance.generated.h"

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSWeaponAimInstance, Log, All);

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSWeaponAnimInstance : public UAnimInstance,public IINSWeaponAnimInterface
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="WeaponRef")
	class AINSWeaponBase* CurrentWeaponRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponRef")
	class UINSWeaponMeshComponent* CurrentWeaponSkeletonRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponRef")
	class UINSWeaponAssets* WeaponAssetsptr;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimDelegate")
		uint8 bWeaponAnimDelegateBindingFinished : 1;
	
protected:
	/** native update for variables tick */
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

	/** native initialize Animation implementation */
	virtual void NativeInitializeAnimation()override;

	virtual bool CheckValid();

public:
	//~begin INSWeaponAnim Interface
	virtual void PlayFireAnim(bool bhasForeGrip,bool bIsDry)override;

	virtual void PlayReloadAnim(bool bHasForeGrip, bool bIsDry)override;

	virtual void PlaySwitchFireModeAnim(bool bHasForeGrip)override;

	virtual void OnWeaponAnimDelegateBindingFinished()override;

	virtual void PlayWeaponBasePose(bool bHasForeGrip)override;

	/** do nothing by default now */
	virtual void PlayAimAnim() {};

	/** do nothing by default now */
	virtual void PlayStopAimAnim() {};

	//~end  INSWeaponAnim Interface

public:
	virtual void PlayWeaponStartEquipAnim(bool bHasForeGrip)override;
};
