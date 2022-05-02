// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_WeaponReady.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSComponents/INSCharSkeletalMeshComponent.h"

UINSAnimNotify_WeaponReady::UINSAnimNotify_WeaponReady()
{
}

void UINSAnimNotify_WeaponReady::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation,EventReference);
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}
	const UClass* const OwnerClass = Owner->GetClass();
	UE_LOG(LogTemp, Log, TEXT("notify mesh comp's owner class name %s"), *OwnerClass->GetName());
	const AINSCharacter* OwnerCharacter = nullptr;
	AINSWeaponBase* OwnerWeapon = nullptr;
	if (OwnerClass->IsChildOf(AINSCharacter::StaticClass()))
	{
		OwnerCharacter = Cast<AINSCharacter>(Owner);
		if (OwnerCharacter)
		{
			OwnerWeapon = OwnerCharacter->GetCurrentWeapon();
			if (OwnerWeapon)
			{
				OwnerWeapon->SetWeaponReady();
				UE_LOG(LogTemp, Log, TEXT("weapon %s Finish Reloading notify triggerd "), *OwnerWeapon->GetName());
			}
		}
	}
	else if (OwnerClass->IsChildOf(AINSWeaponBase::StaticClass()))
	{
		OwnerWeapon = Cast<AINSWeaponBase>(MeshComp->GetOwner());
		if (OwnerWeapon)
		{
			OwnerWeapon->FinishReloadWeapon();
			UE_LOG(LogTemp, Log, TEXT("weapon %s Finish Reloading notify triggerd "), *OwnerWeapon->GetName());
		}
	}
}
