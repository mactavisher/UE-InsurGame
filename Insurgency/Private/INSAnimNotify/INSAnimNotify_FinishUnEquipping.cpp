// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_FinishUnEquipping.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSComponents/INSCharSkeletalMeshComponent.h"
#include "INSCharacter/INSPlayerController.h"

UINSAnimNotify_FinishUnEquipping::UINSAnimNotify_FinishUnEquipping()
{

}

void UINSAnimNotify_FinishUnEquipping::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify::FinishUnEquip Triggered but no owner,abort"));
		return;
	}
	AINSCharacter* OwnerCharacter = Cast<AINSCharacter>(Owner);
	if (OwnerCharacter)
	{
		OwnerCharacter->HandleItemFinishUnEquipRequest();
		UE_LOG(LogTemp, Log, TEXT("Character%s FinishUnEquip notify triggerd and Executed"), *OwnerCharacter->GetName());
	}
}
