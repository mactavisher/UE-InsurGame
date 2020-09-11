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
void UINSAnimNotify_FinishEquipping::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify::FinishEquipping Triggered but no owner,abort"));
		return;
	}
	AINSPlayerController* OwnerPlayer = Cast<AINSPlayerController>(Owner->GetOwner());
	if (OwnerPlayer)
	{
		if (OwnerPlayer->GetLocalRole() == ROLE_Authority&&OwnerPlayer->GetNetMode()!=ENetMode::NM_DedicatedServer)
		{
			OwnerPlayer->SetWeaponState(EWeaponState::IDLE);
		}
		if (OwnerPlayer->GetLocalRole() == ROLE_AutonomousProxy)
		{
			OwnerPlayer->ServerSetWeaponState(EWeaponState::IDLE);
		}
	}
	
}
