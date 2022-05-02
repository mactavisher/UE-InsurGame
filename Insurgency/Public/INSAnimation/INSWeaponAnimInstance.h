// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "INSInterfaces/INSWeaponAnimInterface.h"
#include "Insurgency/Insurgency.h"
#include "INSWeaponAnimInstance.generated.h"

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSWeaponAimInstance, Log, All);

/**
 *
 */
UCLASS()
class INSURGENCY_API UINSWeaponAnimInstance : public UAnimInstance, public IINSWeaponAnimInterface
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponRef")
	class AINSWeaponBase* OwnerWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponRef")
	class UINSWeaponMeshComponent* OwnerWeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponRef")
	class UINSStaticAnimData* WeaponAnimData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CurrentWeaponBaseType")
	EWeaponBasePoseType CurrentWeaponBasePoseType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimDelegate")
	uint8 bWeaponAnimDelegateBindingFinished : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Optic")
	uint8 bOpticEquipped:1;


protected:
	/** native update for variables tick */
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** native initialize Animation implementation */
	virtual void NativeInitializeAnimation() override;

	virtual bool CheckValid();

public:
	//~begin INSWeaponAnim Interface
	virtual float PlayFireAnim() override;
	virtual void SetOpticEquipped(bool OpticEquipped) { bOpticEquipped = OpticEquipped; };
	virtual float PlayReloadAnim(bool bIsDry) override;
	virtual float PlaySwitchFireModeAnim() override;
	virtual void OnWeaponAnimDelegateBindingFinished() override;
	virtual float PlayWeaponBasePose() override;
	virtual float PlayWeaponStartUnEquipAnim() override;
	virtual void SetWeaponBasePoseType(EWeaponBasePoseType NewWeaponBasePoseType);
	virtual float PlayAimAnim() { return 0.f; };
	virtual float PlayStopAimAnim() { return 0.f; };
	//~end  INSWeaponAnim Interface

public:
	virtual float PlayWeaponStartEquipAnim() override;
};
