// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "INSStaticAnimData.generated.h"

class UAnimMontage;
class UAnimationAsset;
class UBlendSpace;
class UBlendSpace1D;
class UAnimSequence;
class UAimOffsetBlendSpace;
class UAimOffsetBlendSpace1D;

/**
 * custom weapon animation pair
 * once for character and one for weapon self
 */
USTRUCT(BlueprintType)
struct FCustomWeaponAnim
{
	GENERATED_USTRUCT_BODY()
		//shared additive animation between different weapons
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UAnimMontage* CharAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UAnimMontage* WeaponAnim;
};

USTRUCT(BlueprintType)
struct FWeaponBasePose
{
	GENERATED_USTRUCT_BODY()
		//shared additive animation between different weapons
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UAnimMontage* CharAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UAnimMontage* WeaponAnim;
};


/**
 * default pose animations
 */
USTRUCT(BlueprintType)
struct FCustomDefaultPoseWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim DeployAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim ReloadAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim ReloadDryAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim UnDeployAnim;
};

/**
 * alt grip animations
 */
USTRUCT(BlueprintType)
struct FCustomAltGripWeaponAnim
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim DeployAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim ReloadAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim ReloadDryAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim UnDeployAnim;
};

/**
 * ForeGrip animations
 */
USTRUCT(BlueprintType)
struct FCustomForeGripWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim DeployAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim ReloadAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim ReloadDryAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FCustomWeaponAnim UnDeployAnim;
};

UCLASS(abstract,BlueprintType)
class INSURGENCY_API UINSStaticAnimData : public UObject
{
	GENERATED_BODY()
	friend class UINSCharacterAimInstance;
	friend class UINSWeaponAnimInstance;

public:

	//shared additive animation between different weapons
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		UAnimMontage* FPIdleAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		UAnimMontage* FPAimIdleAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		UAnimMontage* FPMoveAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		UAnimMontage* FPAimMoveAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		UAnimMontage* FPSprintAnim;

	/** weapon fire trigger and bolt animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		UAnimMontage* WeaponFireAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		TArray<UAnimMontage*> FPFireHandsRecoilAnims;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		TArray<UAnimMontage*> FPAdsFireHandsSmallCalibers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		TArray<UAnimMontage*> FPAdsFireHandsMediumCalibers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|SharedAddtive")
		TArray<UAnimMontage*> FPAdsFireHandsHighCalibers;

	//shared additive animation between different weapons
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim")
		FWeaponBasePose FPDefaultBasePose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim")
		FWeaponBasePose FPAltGripBasePose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim")
		FWeaponBasePose FPForeGripBasePose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim")
		FCustomWeaponAnim FPPulltriggerAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|Ads")
		UAnimSequence* AdjustableAdsRefPose;

	//FP default pose
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|CustomWeaponAnim")
		FCustomDefaultPoseWeaponAnim FPWeaponDefaultPoseAnim;

	//FP alt grip pose
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|CustomWeaponAnim")
		FCustomAltGripWeaponAnim FPWeaponAltGripAnim;

	//FP fore grip pose
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPAnim|CustomWeaponAnim")
		FCustomForeGripWeaponAnim FPWeaponForeGripAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|SharedAddtive")
		TArray<UAnimMontage*> TPFireRecoilAnims;

	//shared additive animation between different weapons
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim")
		FWeaponBasePose TPDefaultBasePose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim")
		FWeaponBasePose TPAltGripBasePose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim")
		FWeaponBasePose TPForeGripBasePose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim")
		FCustomWeaponAnim TPPulltriggerAnim;

	//TP default pose CustomAinm
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|CustomWeaponAnim")
		FCustomDefaultPoseWeaponAnim TPWeaponDefaultPoseAnim;

	//TP alt grip pose CustomAinm
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|CustomWeaponAnim")
		FCustomAltGripWeaponAnim TPWeaponAltGripAnim;

	//TP fore grip pose CustomAinm
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|CustomWeaponAnim")
		FCustomForeGripWeaponAnim TPWeaponForeGripAnim;

	//TP StandJogBlendSpace
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|BlendSpaces|Stand")
		UBlendSpace* StandJogBlendSpace;

	//TP StandWalkBlendSpace,when ads
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|BlendSpaces|Stand")
		UBlendSpace* StandWalkBlendSpace;

	//TP StandAimOffSet
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|BlendSpaces|Stand")
		UAimOffsetBlendSpace1D* StandAimOffSet;

	//TP CrouchMoveBlendSpace
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|BlendSpaces|Crouch")
		UBlendSpace* CrouchMoveBlendSpace;

	//TP StandAimOffSet
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|BlendSpaces|Stand")
		UAimOffsetBlendSpace1D* CrouchAimOffSet;

	//TP ProneMoveBlendSpace
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TPAnim|BlendSpaces|Prone")
		UBlendSpace* ProneMoveBlendSpace;
};