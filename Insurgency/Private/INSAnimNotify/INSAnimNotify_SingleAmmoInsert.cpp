// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_SingleAmmoInsert.h"

#include "INSCharacter/INSCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

UINSAnimNotify_SingleAmmoInsert::UINSAnimNotify_SingleAmmoInsert()
{
	
}

void UINSAnimNotify_SingleAmmoInsert::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify::SingleAmmoInsert Triggered but no owner,abort"));
		return;
	}
	const UClass* const OwnerClass = Owner->GetClass();
	UE_LOG(LogTemp, Log, TEXT("notify mesh comp's owner class name %s"), *OwnerClass->GetName());
	if (OwnerClass->IsChildOf(AINSCharacter::StaticClass()))
	{
		AINSCharacter* OwnerCharacter = Cast<AINSCharacter>(Owner);
		if (OwnerCharacter)
		{
			OwnerCharacter->HandleSingleAmmoInsertRequest();
			UE_LOG(LogTemp, Log, TEXT("Character%s Finish SingleAmmoInsert notify triggerd and Executed"), *OwnerCharacter->GetName());
		}
	}
}
