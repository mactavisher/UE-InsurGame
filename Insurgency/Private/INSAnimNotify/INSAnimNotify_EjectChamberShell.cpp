// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_EjectChamberShell.h"

#include "INSItems/INSWeapons/INSWeaponBase.h"

UINSAnimNotify_EjectChamberShell::UINSAnimNotify_EjectChamberShell()
{
}

void UINSAnimNotify_EjectChamberShell::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation,EventReference);
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify::FinishEquipping Triggered but no owner,abort"));
		return;
	}
	if (Owner->GetWorld() && Owner->IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	AINSWeaponBase* OwnerWeapon = Cast<AINSWeaponBase>(Owner);
	if (OwnerWeapon && OwnerWeapon->HasShellLeftInChamber())
	{
		OwnerWeapon->CastProjectileShell();
		UE_LOG(LogTemp, Log, TEXT("Weapon%s CastProjectileShell notify triggerd and Executed"), *OwnerWeapon->GetName());
	}
}
