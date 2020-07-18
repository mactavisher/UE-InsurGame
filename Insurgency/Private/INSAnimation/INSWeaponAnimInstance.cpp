// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSWeaponAnimInstance.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSAssets/INSWeaponAssets.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogINSWeaponAimInstance);

UINSWeaponAnimInstance::UINSWeaponAnimInstance(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bWeaponAnimDelegateBindingFinished = false;
}

void UINSWeaponAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (CurrentWeaponRef)
	{
		PlayWeaponBasePose(CurrentWeaponRef->bForeGripEquipt);
	}
}

void UINSWeaponAnimInstance::NativeInitializeAnimation()
{
	CurrentWeaponSkeletonRef = Cast<UINSWeaponMeshComponent>(GetSkelMeshComponent());
	if (CurrentWeaponSkeletonRef)
	{
		CurrentWeaponRef = Cast<AINSWeaponBase>(CurrentWeaponSkeletonRef->GetOwner());
		WeaponAssetsptr = CurrentWeaponRef->GetWeaponAssets();
	}
	if (CurrentWeaponRef && !bWeaponAnimDelegateBindingFinished)
	{
		BindWeaponAnimDelegate();
	}
}

void UINSWeaponAnimInstance::PlayFireAnim(bool bhasForeGrip, bool bIsDry)
{
	if (!CheckValid())
	{
		return;
	}
	//never play fire visual animation on dedicated server side 
	if (CurrentWeaponRef->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}
	Montage_Play(WeaponAssetsptr->FireAnimFPTP.GunFireMontage);
}

void UINSWeaponAnimInstance::PlayReloadAnim(bool bHasForeGrip, bool bIsDry)
{
	if (!CheckValid())
	{
		return;
	}
	//never play reload visual animation on dedicated server side 
	if (CurrentWeaponRef->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}
	UAnimMontage* SelectedGunReloadMontage = nullptr;
	if (bHasForeGrip && !bIsDry)
	{
		SelectedGunReloadMontage = WeaponAssetsptr->ReloadForeGripFP.GunReloadMontage;
	}
	if (bHasForeGrip && bIsDry)
	{
		SelectedGunReloadMontage = WeaponAssetsptr->ReloadDryForeGripFP.GunReloadDryMontage;
	}
	if (!bHasForeGrip && !bIsDry)
	{
		SelectedGunReloadMontage = WeaponAssetsptr->ReloadAltGripFP.GunReloadMontage;
	}
	if (!bHasForeGrip && bIsDry)
	{
		SelectedGunReloadMontage = WeaponAssetsptr->ReloadDryAltGripFP.GunReloadDryMontage;
	}
	if (!SelectedGunReloadMontage)
	{
		UE_LOG(LogINSWeaponAimInstance, Warning, TEXT("weapon %s is trying to play Reload montage,but selectd reload Montage is missing,abort!!!"), *CurrentWeaponRef->GetName());
		return;
	}
	Montage_Play(SelectedGunReloadMontage);
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("weapon:%s Is playing Reload montage,reload Montage Name is %s"), *CurrentWeaponRef->GetName(), *SelectedGunReloadMontage->GetName());
}

void UINSWeaponAnimInstance::PlaySwitchFireModeAnim(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	//never play reload visual animation on dedicated server side 
	if (CurrentWeaponRef->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}
	UAnimMontage* SelectedGunFireModeSwitchAnim = nullptr;
	if (bHasForeGrip)
	{
		SelectedGunFireModeSwitchAnim = WeaponAssetsptr->FireModeSwitchAltGripFP.GunSwitchFireModeMontage;
	}
	else
	{
		SelectedGunFireModeSwitchAnim= WeaponAssetsptr->FireModeSwitchAltGripFP.GunSwitchFireModeMontage;
	}
	if (!SelectedGunFireModeSwitchAnim)
	{
		UE_LOG(LogINSWeaponAimInstance, Warning, TEXT("weapon %s is trying to play Reload montage,but selectd reload Montage is missing,abort!!!"), *CurrentWeaponRef->GetName());
		return;
	}
	Montage_Play(SelectedGunFireModeSwitchAnim);
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("weapon:%s Is playing Reload montage,reload Montage Name is %s"), *CurrentWeaponRef->GetName(), *SelectedGunFireModeSwitchAnim->GetName());
}

void UINSWeaponAnimInstance::OnWeaponAnimDelegateBindingFinished()
{
	bWeaponAnimDelegateBindingFinished = true;
	PlayWeaponStartEquipAnim(CurrentWeaponRef->bForeGripEquipt);
}

void UINSWeaponAnimInstance::PlayWeaponBasePose(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	UAnimMontage* SelectedWeaponBasePose = nullptr;
	bHasForeGrip = CurrentWeaponRef->bForeGripEquipt;
	SelectedWeaponBasePose = bHasForeGrip ? WeaponAssetsptr->BasePoseForeGripFP.GunBasePoseMontage : WeaponAssetsptr->BasePoseAltGripFP.GunBasePoseMontage;
}

void UINSWeaponAnimInstance::PlayWeaponStartEquipAnim(bool bHasForeGrip)
{
	if (!CheckValid())
	{
		return;
	}
	//never play equip visual animation on dedicated server side 
	if (CurrentWeaponRef->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}
	bHasForeGrip = CurrentWeaponRef->bForeGripEquipt;
	UAnimMontage* SelectedGunEquipMontage = nullptr;
	if (bHasForeGrip)
	{
		SelectedGunEquipMontage = WeaponAssetsptr->EquipForeGripFP.GunEquipMontage;
	}
	else
	{
		SelectedGunEquipMontage = WeaponAssetsptr->EquipAltGripFP.GunEquipMontage;
	}
	if (!SelectedGunEquipMontage)
	{
		UE_LOG(LogINSWeaponAimInstance, Warning, TEXT("weapon %s is trying to play equip montage,but selectd reload Montage is missing,abort!!!"), *CurrentWeaponRef->GetName());
		return;
	}
	Montage_Play(SelectedGunEquipMontage);
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("weapon:%s Is playing equip montage,reload Montage Name is %s"), *CurrentWeaponRef->GetName(), *SelectedGunEquipMontage->GetName());
}

bool UINSWeaponAnimInstance::CheckValid()
{
	if (!bWeaponAnimDelegateBindingFinished)
	{
		UE_LOG(LogINSWeaponAimInstance, Warning, TEXT("waiting for delegate binding finished,can't play animations"));
		return false;
	}
	if (!CurrentWeaponRef)
	{
		UE_LOG(LogINSWeaponAimInstance, Warning, TEXT("Missing Current Weapon Ref,invalid for playing any weapon anim"));
		return false;
	}
	if (!WeaponAssetsptr)
	{
		UE_LOG(LogINSWeaponAimInstance, Warning, TEXT("Missing Current Weapon asstes Ref,invalid for playing any weapon anim"));
		return false;
	}
	return true;
}

void UINSWeaponAnimInstance::BindWeaponAnimDelegate()
{
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("Start Binding Weapon Anim Delegate for Weapon"));
	FireDelegate.BindUFunction(this, TEXT("PlayFireAnim"));
	StartSwitchFireModeDelegate.BindUFunction(this, TEXT("PlaySwitchFireModeAnim"));
	StartEquipDelegate.BindUFunction(this, TEXT("PlayWeaponStartEquipAnim"));
	StartReloadDelegate.BindUFunction(this, TEXT("PlayReloadAnim"));
	CurrentWeaponRef->OnWeaponStartEquip.AddUnique(StartEquipDelegate);
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("Bind Weapon Equip Anim delegate for Weapon: Delegate Function::PlayWeaponStartEquipAnim"));
	CurrentWeaponRef->OnWeaponEachFire.AddUnique(FireDelegate);
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("Bind Weapon Fire Anim delegate for Weapon: Delegate Function::PlayFireAnim"));
	CurrentWeaponRef->OnWeaponStartReload.AddUnique(StartReloadDelegate);
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("Bind Weapon Reload Anim delegate for Weapon: Delegate Function::PlayReloadAnim"));
	CurrentWeaponRef->OnWeaponSwitchFireMode.AddUnique(StartSwitchFireModeDelegate);
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("Bind Weapon Switch Fire Mode Anim delegate for Weapon: Delegate Function::PlaySwitchFireModeAnim"));
	UE_LOG(LogINSWeaponAimInstance, Log, TEXT("Finish Binding Weapon Anim Delegate for Weapon"));
	OnWeaponAnimDelegateBindingFinished();
}

void UINSWeaponAnimInstance::UnbindWeaponAnimDelegate()
{

}

