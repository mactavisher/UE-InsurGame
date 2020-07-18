// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_FinishUnEquipping.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSComponents/INSCharSkeletalMeshComponent.h"

UINSAnimNotify_FinishUnEquipping::UINSAnimNotify_FinishUnEquipping()
{

}

void UINSAnimNotify_FinishUnEquipping::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify::FinishUnEquipping Triggered but no owner,abort"));
		return;
	}
	const UClass* const OwnerClass = Owner->GetClass();
	UE_LOG(LogTemp, Log, TEXT("notify mesh comp's owner class name %s"), *OwnerClass->GetName());
	const AINSCharacter* OwnerCharacter = nullptr;
	AINSWeaponBase* OwnerWeapon = nullptr;
	if (OwnerClass->IsChildOf(AINSCharacter::StaticClass()))
	{
		OwnerCharacter = Cast<AINSCharacter>(Owner);
		if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
		{
			OwnerWeapon = OwnerCharacter->GetCurrentWeapon();
			if (OwnerWeapon)
			{
				OwnerWeapon->FinishEquippingWeapon();
				UE_LOG(LogTemp, Log, TEXT("weapon %s FinishUnEquipping notify triggerd and Executed"), *OwnerWeapon->GetName());
			}
		}
	}
	else if (OwnerClass->IsChildOf(AINSWeaponBase::StaticClass()))
	{
		OwnerWeapon = Cast<AINSWeaponBase>(MeshComp->GetOwner());
		if (OwnerWeapon && OwnerWeapon->GetIsOwnerLocal())
		{
			OwnerWeapon->FinishEquippingWeapon();
			UE_LOG(LogTemp, Log, TEXT("weapon %s FinishUnEquipping notify triggerd and Executed"), *OwnerWeapon->GetName());
		}
	}
}
