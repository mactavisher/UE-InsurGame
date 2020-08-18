// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSWeaponAttachments/INSWeaponAttachment.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "GameFramework/PlayerController.h"
#include "INSCharacter/INSCharacter.h"
#include "Net/UnrealNetwork.h"

AINSWeaponAttachment::AINSWeaponAttachment(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.SetTickFunctionEnable(false);
}

void AINSWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();

}

void AINSWeaponAttachment::OnRep_OwnerWeapon()
{

}


class AController* AINSWeaponAttachment::GetOwnerPlayer()
{
	if (WeaponOwner)
	{
		return WeaponOwner->GetOwnerCharacter()->GetController();
	}
	return nullptr;
}

class AController* AINSWeaponAttachment::GetOwingPlayer()
{
	if (WeaponOwner)
	{
		const AINSCharacter* const OwningCharacter = WeaponOwner->GetOwnerCharacter();
		if (OwningCharacter)
		{
			return OwningCharacter->GetController();
		}
		return nullptr;
	}
	return nullptr;
}

void AINSWeaponAttachment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSWeaponAttachment, WeaponOwner);
}

void AINSWeaponAttachment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

