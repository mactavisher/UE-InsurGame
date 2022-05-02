// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSWeaponAttachments/INSWeaponAttachment.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#ifndef UINSWeaponMeshComponent
#include "INSComponents/INSWeaponMeshComponent.h"
#endif
#include "INSItems/INSWeapons/INSWeaponBase.h"

AINSWeaponAttachment::AINSWeaponAttachment(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	AttachmentMeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AttachmentMeshComp"));
	AttachmentMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachmentMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RootComponent = AttachmentMeshComp;
	AttachmentMeshComp->AlwaysLoadOnClient = true;
	AttachmentMeshComp->SetHiddenInGame(false);
	bChangeWeaponBasePoseType = false;
	SetReplicatingMovement(false);
	ItemType = EItemType::WEAPONATTACHEMENT;
	AttachmentType = EWeaponAttachmentType::NONE;
	AttachedSlotIndex = static_cast<uint8>(0);
	bBlockQuickBoltRifileReloading = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	TargetFOV = 70.f;
}

void AINSWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();
	if (WeaponOwner)
	{
		if (WeaponOwner->GetIsClientCosmeticWeapon())
		{
			SetActorHiddenInGame(true);
			AttachmentMeshComp->bCastHiddenShadow = true;
		}
		else
		{
			if (WeaponOwner->GetLocalRole() >= ROLE_AutonomousProxy)
			{
				AttachmentMeshComp->SetCastShadow(false);
				AttachmentMeshComp->SetCastHiddenShadow(false);
			}
		}
	}
}

void AINSWeaponAttachment::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AINSWeaponAttachment::OnRep_Owner()
{
	Super::OnRep_Owner();
}

void AINSWeaponAttachment::AttachToWeaponSlot()
{
	if (!WeaponOwner)
	{
		return;
	}
	AttachmentMeshComp->AttachToComponent(GetWeaponOwner()->GetWeaponMeshComp(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName(TEXT("Optic")));
}

void AINSWeaponAttachment::OnRep_OwnerWeapon()
{
	if (!WeaponOwner)
	{
		return;
	}
	AttachToWeaponSlot();
	WeaponOwner->AddAttachmentInstance(this);
}


void AINSWeaponAttachment::CheckAndUpdateWeaponBasePoseType()
{
	if (bChangeWeaponBasePoseType && WeaponOwner)
	{
		WeaponOwner->SetWeaponBasePoseType(TargetWeaponBasePoseType);
	}
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


void AINSWeaponAttachment::ReceiveAttachmentEquipped(class AINSWeaponBase* WeaponEquippedBy)
{
	this->WeaponOwner = WeaponEquippedBy;
}

void AINSWeaponAttachment::SetWeaponOwner(class AINSWeaponBase* NewWeaponOwner)
{
	this->WeaponOwner = NewWeaponOwner;
	if (HasAuthority())
	{
		OnRep_OwnerWeapon();
	}
}

FORCEINLINE class UStaticMeshComponent* AINSWeaponAttachment::GetAttachmentMeshComp() const
{
	return AttachmentMeshComp;
}
