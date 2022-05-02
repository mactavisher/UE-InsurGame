// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_FinishReloading.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSComponents/INSCharSkeletalMeshComponent.h"

UINSAnimNotify_FinishReloading::UINSAnimNotify_FinishReloading()
{
}

void UINSAnimNotify_FinishReloading::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation,EventReference);
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify::FinishReloading Triggered but no owner,abort"));
		return;
	}
	AINSCharacter* OwnerCharacter = Cast<AINSCharacter>(Owner);
	if (OwnerCharacter)
	{
		OwnerCharacter->HandleFinishReloadingRequest();
		UE_LOG(LogTemp, Log, TEXT("Character%s FinishReloading notify triggerd and Executed"), *OwnerCharacter->GetName());
	}
}
