// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "INSWeaponAssets.generated.h"

class UAnimMontage;
class UBlendSpace;
class UAnimationAsset;
class UTexture2D;

//montages used for FPS characters
USTRUCT(BlueprintType)
struct FCharMovementAnim1p
{
	GENERATED_USTRUCT_BODY()
	/** move montage to use, walk montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
		UAnimMontage* MoveMontage;

	/** sprint montage to use, walk montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
		UAnimMontage* SprintMontage;

	/** move montage to use, walk montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
		UAnimMontage* AimMoveMontage;
};
USTRUCT(BlueprintType)
struct FBasePoseAnim
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
		UAnimMontage* CharBasePoseMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
		UAnimMontage* GunBasePoseMontage;
};

USTRUCT(BlueprintType)
struct FFireAddtitiveAnimFPTP
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Fire")
	     UAnimMontage* CharPullTriggerMontage;
	/** used to blend arm shaking when shoot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
		TArray<UAnimMontage*> CharFireRecoilMontages;

	/** used for blend arm shaking when ADS shoot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
		TArray<UAnimMontage*> CharFireRecoilMontagesADS;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Fire")
	    UAnimMontage* GunFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	    UAnimMontage* FireSwayAim;
};

USTRUCT(BlueprintType)
struct FFireModeSwitchAnim
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FireModeSwitch")
		UAnimMontage* CharSwitchFireModeMontage;

	/** used to blend finger pull trigger */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FireModeSwitch")
		UAnimMontage* GunSwitchFireModeMontage;
};

USTRUCT(BlueprintType)
struct FAimAnim
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aim")
		UAnimSequence* AdjustableAimRef;

	/** used to blend finger pull trigger */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aim")
		UAnimMontage* GunAimAnim;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Aim")
	   float AimTime;
};

USTRUCT(BlueprintType)
struct FReloadAnim
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Reload")
		UAnimMontage* CharReloadModeMontage;

	/** used to blend finger pull trigger */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Reload")
		UAnimMontage* GunReloadMontage;
};

USTRUCT(BlueprintType)
struct FReloadDryAnim
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ReloadDry")
		UAnimMontage* CharReloadDryModeMontage;

	/** used to blend finger pull trigger */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ReloadDry")
		UAnimMontage* GunReloadDryMontage;
};

USTRUCT(BlueprintType)
struct FEquipAnim
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ReloadDry")
		UAnimMontage* CharEquipMontage;

	/** used to blend finger pull trigger */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ReloadDry")
		UAnimMontage* GunEquipMontage;
};

USTRUCT(BlueprintType)
struct FIdleAnim
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ReloadDry")
		UAnimMontage* CharIdleMontage;
};



UCLASS()
class INSURGENCY_API UINSWeaponAssets : public UDataAsset
{
	GENERATED_BODY()
	friend class AINSWeaponBase;
	friend class UINSCharacterAimInstance;
	friend class UINSWeaponAnimInstance;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FCharMovementAnim1p MoveAnim1P;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FFireAddtitiveAnimFPTP FireAnimFPTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FIdleAnim IdleAnimFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FIdleAnim IdleAnimTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FBasePoseAnim BasePoseAltGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FBasePoseAnim BasePoseAltGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FBasePoseAnim BasePoseForeGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FBasePoseAnim BasePoseForeGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FAimAnim AimAnimFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FAimAnim AimAnimTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FFireModeSwitchAnim FireModeSwitchForeGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FFireModeSwitchAnim FireModeSwitchForeGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FFireModeSwitchAnim FireModeSwitchAltGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FFireModeSwitchAnim FireModeSwitchAltGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FReloadAnim ReloadAltGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FReloadAnim ReloadAltGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FReloadAnim ReloadForeGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FReloadAnim ReloadForeGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FReloadDryAnim ReloadDryAltGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FReloadDryAnim ReloadDryAltGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FReloadDryAnim ReloadDryForeGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FReloadDryAnim ReloadDryForeGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FEquipAnim EquipAltGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FEquipAnim EquipAltGripTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FP")
		FEquipAnim EquipForeGripFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TP")
		FEquipAnim EquipForeGripTP;

	/** TP blendSpaces */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "3P|BlendSpace")
		UBlendSpace* StandWalkType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "3P|BlendSpace")
		UBlendSpace* CrouchWalkType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "3P|BlendSpace")
		UBlendSpace* ProneMoveType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "3P|BlendSpace")
		UBlendSpace* StandJogType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "3P|BlendSpace")
		UBlendSpace* CrouchJogType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|WeaponIcon")
		UTexture2D* WeaponIcon;
};
