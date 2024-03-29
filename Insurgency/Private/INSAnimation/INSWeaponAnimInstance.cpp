// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSWeaponAnimInstance.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSAssets/INSStaticAnimData.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogINSWeaponAimInstance);

UINSWeaponAnimInstance::UINSWeaponAnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bWeaponAnimDelegateBindingFinished = false;
	bOpticEquipped = false;
}

void UINSWeaponAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	OwnerWeaponMesh = Cast<UINSWeaponMeshComponent>(GetSkelMeshComponent());
	if (OwnerWeaponMesh)
	{
		OwnerWeapon = Cast<AINSWeaponBase>(OwnerWeaponMesh->GetOwner());
		WeaponAnimData = OwnerWeapon->GetWeaponAnimDataPtr();
	}
	if (OwnerWeapon)
	{
		PlayWeaponBasePose();
	}
}

void UINSWeaponAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

float UINSWeaponAnimInstance::PlayFireAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedPullTriggerAnim = nullptr;
	switch (CurrentWeaponBasePoseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedPullTriggerAnim =
			WeaponAnimData->FPWeaponAltGripAnim.PullTriggerAnim.WeaponAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedPullTriggerAnim =
			WeaponAnimData->FPWeaponForeGripAnim.PullTriggerAnim.WeaponAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedPullTriggerAnim = WeaponAnimData->FPWeaponDefaultPoseAnim.
	                                                                             PullTriggerAnim.WeaponAnim;
		break;
	default: SelectedPullTriggerAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedPullTriggerAnim);
}


float UINSWeaponAnimInstance::PlayReloadAnim(bool bIsDry)
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedReloadAnim = nullptr;
	switch (CurrentWeaponBasePoseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedReloadAnim = bIsDry
		                                                        ? WeaponAnimData->FPWeaponAltGripAnim.ReloadDryAnim.
		                                                                          WeaponAnim
		                                                        : WeaponAnimData->FPWeaponAltGripAnim.ReloadAnim.
		                                                                          WeaponAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedReloadAnim = bIsDry
		                                                         ? WeaponAnimData->FPWeaponForeGripAnim.ReloadDryAnim.
		                                                                           WeaponAnim
		                                                         : WeaponAnimData->FPWeaponForeGripAnim.ReloadAnim.
		                                                                           WeaponAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedReloadAnim = bIsDry
		                                                        ? WeaponAnimData->FPWeaponDefaultPoseAnim.ReloadDryAnim.
		                                                                          WeaponAnim
		                                                        : WeaponAnimData->FPWeaponDefaultPoseAnim.ReloadAnim.
		                                                                          WeaponAnim;
		break;
	default: SelectedReloadAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedReloadAnim);
}

float UINSWeaponAnimInstance::PlaySwitchFireModeAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	return 0.f;
}

void UINSWeaponAnimInstance::OnWeaponAnimDelegateBindingFinished()
{
	bWeaponAnimDelegateBindingFinished = true;
	PlayWeaponStartEquipAnim();
}

float UINSWeaponAnimInstance::PlayWeaponBasePose()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedBasePoseAnim = nullptr;
	switch (CurrentWeaponBasePoseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedBasePoseAnim = WeaponAnimData->FPWeaponAltGripAnim.BasePoseAnim.WeaponAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedBasePoseAnim = WeaponAnimData->FPWeaponForeGripAnim.BasePoseAnim.WeaponAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedBasePoseAnim = WeaponAnimData->FPWeaponDefaultPoseAnim.BasePoseAnim.WeaponAnim;
		break;
	default: SelectedBasePoseAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedBasePoseAnim);
}

float UINSWeaponAnimInstance::PlayWeaponStartUnEquipAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedUnEquipAnim = nullptr;
	switch (CurrentWeaponBasePoseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedUnEquipAnim = WeaponAnimData->FPWeaponAltGripAnim.UnDeployAnim.WeaponAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedUnEquipAnim = WeaponAnimData->FPWeaponForeGripAnim.UnDeployAnim.WeaponAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedUnEquipAnim = WeaponAnimData->FPWeaponDefaultPoseAnim.UnDeployAnim.WeaponAnim;
		break;
	default: SelectedUnEquipAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedUnEquipAnim);
}

void UINSWeaponAnimInstance::SetWeaponBasePoseType(EWeaponBasePoseType NewWeaponBasePoseType)
{
	CurrentWeaponBasePoseType = NewWeaponBasePoseType;
}

float UINSWeaponAnimInstance::PlayWeaponStartEquipAnim()
{
	if (!CheckValid())
	{
		return 0.f;
	}
	UAnimMontage* SelectedEquipAnim = nullptr;
	switch (CurrentWeaponBasePoseType)
	{
	case EWeaponBasePoseType::ALTGRIP: SelectedEquipAnim = WeaponAnimData->FPWeaponAltGripAnim.DeployAnim.WeaponAnim;
		break;
	case EWeaponBasePoseType::FOREGRIP: SelectedEquipAnim = WeaponAnimData->FPWeaponForeGripAnim.DeployAnim.WeaponAnim;
		break;
	case EWeaponBasePoseType::DEFAULT: SelectedEquipAnim = WeaponAnimData->FPWeaponDefaultPoseAnim.DeployAnim.WeaponAnim;
		break;
	default: SelectedEquipAnim = nullptr;
		break;
	}
	return Montage_Play(SelectedEquipAnim);
}

bool UINSWeaponAnimInstance::CheckValid()
{
	if (!OwnerWeapon)
	{
		//UE_LOG(LogINSWeaponAimInstance, Warning, TEXT("Missing Current Weapon Ref,invalid for playing any weapon anim"));
		return false;
	}
	if (!WeaponAnimData)
	{
		//UE_LOG(LogINSWeaponAimInstance, Warning , TEXT("Missing Current Weapon asstes Ref,invalid for playing any weapon anim"));
		return false;
	}
	return true;
}
