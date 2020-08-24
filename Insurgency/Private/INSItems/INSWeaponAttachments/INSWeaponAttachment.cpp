// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSWeaponAttachments/INSWeaponAttachment.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

AINSWeaponAttachment::AINSWeaponAttachment(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	SetReplicates(true);
	Mesh1p=ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this,TEXT("Mesh1pComp"));
	Mesh3p = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh3pComp"));
	Mesh1p->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3p->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1p->AlwaysLoadOnClient = true;
	Mesh3p->AlwaysLoadOnServer = true;
	Mesh1p->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh3p->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemType = EItemType::WEAPONATTACHMENT;
}

void AINSWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();
	DisableTick();
}

void AINSWeaponAttachment::OnRep_Owner()
{
	Super::OnRep_Owner();
	class AINSPlayerController* const OwnerPlayer = GetOwnerPlayer<AINSPlayerController>();
	const class AINSWeaponBase* PlayerCurrentWeapon = nullptr;
	if (OwnerPlayer)
	{
		if (OwnerPlayer->IsLocalController())
		{
			Mesh1p->SetHiddenInGame(false);
			Mesh3p->SetHiddenInGame(true);
		}
		PlayerCurrentWeapon = OwnerPlayer->GetINSPlayerCharacter()->GetCurrentWeapon();
	}
	//we can't set the owner weapon to this current weapon yet,because this attachment may not equipped yet
}


void AINSWeaponAttachment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AINSWeaponAttachment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AINSWeaponAttachment::ReceiveAttachmentEquipped(class AINSWeaponBase* WeaponEuippedBy)
{
	this->WeaponOwner = WeaponEuippedBy;
}

