// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_FinishEquipping.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#ifndef  AINSPlayerController
#include "INSCharacter/INSPlayerController.h"
#endif
#include "INSComponents/INSCharSkeletalMeshComponent.h"

UINSAnimNotify_FinishEquipping::UINSAnimNotify_FinishEquipping()
{
}

void UINSAnimNotify_FinishEquipping::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify::FinishEquipping Triggered but no owner,abort"));
		return;
	}
	AINSCharacter* OwnerCharacter = Cast<AINSCharacter>(Owner);
	if (OwnerCharacter)
	{
		OwnerCharacter->HandleItemFinishEquipRequest();
		UE_LOG(LogTemp, Log, TEXT("Character%s FinishEquip notify triggerd and Executed"), *OwnerCharacter->GetName());
	}
}
