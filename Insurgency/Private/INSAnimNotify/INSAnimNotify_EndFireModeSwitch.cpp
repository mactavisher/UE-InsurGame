// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_EndFireModeSwitch.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSComponents/INSCharSkeletalMeshComponent.h"

UINSAnimNotify_EndFireModeSwitch::UINSAnimNotify_EndFireModeSwitch()
{
}

void UINSAnimNotify_EndFireModeSwitch::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify::EndFireModeSwitch Triggered but no owner , abort"));
		return;
	}
	const UClass* const OwnerClass = Owner->GetClass();
	UE_LOG(LogTemp, Log, TEXT("notify mesh comp's owner class name %s"), *OwnerClass->GetName());
	if (OwnerClass->IsChildOf(AINSCharacter::StaticClass()))
	{
		const AINSCharacter* OwnerCharacter = Cast<AINSCharacter>(Owner);
		AINSWeaponBase* OwnerWeapon = OwnerCharacter->GetCurrentWeapon();
		if (OwnerWeapon)
		{
			if (OwnerWeapon->HasAuthority())
			{
				OwnerWeapon->FinishSwitchFireMode();
			}
			else if (OwnerWeapon->GetLocalRole() == ROLE_AutonomousProxy)
			{
				OwnerWeapon->ServerFinishSwitchFireMode();
			}
			UE_LOG(LogTemp, Log, TEXT("weapon %s FinishUnEquipping notify triggerd and Executed"),
			       *OwnerWeapon->GetName());
		}
	}
}
