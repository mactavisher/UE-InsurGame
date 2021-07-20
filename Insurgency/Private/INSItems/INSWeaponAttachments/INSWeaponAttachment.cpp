// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSWeaponAttachments/INSWeaponAttachment.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#ifndef UINSWeaponMeshComponent
#include "INSComponents/INSWeaponMeshComponent.h"
#endif
#include "INSItems/INSWeapons/INSWeaponBase.h"

AINSWeaponAttachment::AINSWeaponAttachment(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bReplicates = true;
	AttachmentMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh3pComp"));
	AttachmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachmentMesh->AlwaysLoadOnClient = true;
	bChangeWeaponBasePoseType = false;
	SetReplicatingMovement(false);
	ItemType = EItemType::WEAPONATTACHMENT;
	bClientVisualAttachment = false;
	DisableTick();
}

void AINSWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();
	if (!bClientVisualAttachment)
	{
		SetActorHiddenInGame(true);
		UpdateWeaponBasePoseType();
		AttachToWeaponSlot();
	}
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
	if ((WeaponOwner->GetLocalRole() == ROLE_AutonomousProxy)
		|| (HasAuthority() && !bClientVisualAttachment))
	{
		GetClientVisualAttachment()->GetAttachmentMeshComp()->AttachToComponent(WeaponOwner->WeaponMesh1PComp, FAttachmentTransformRules::KeepRelativeTransform, NAME_None);
	}
	else
	{
		GetClientVisualAttachment()->GetAttachmentMeshComp()->AttachToComponent(WeaponOwner->WeaponMesh3PComp, FAttachmentTransformRules::KeepRelativeTransform, NAME_None);
	}
}

void AINSWeaponAttachment::OnRep_OwnerWeapon()
{
	if (!WeaponOwner)
	{
		return;
	}
	CreateClientVisualAttachment();
	AttachToWeaponSlot();
}

void AINSWeaponAttachment::UpdateWeaponBasePoseType()
{
	if (bChangeWeaponBasePoseType)
	{
		WeaponOwner->SetWeaponBasePoseType(TargetWeaponBasePoseType);
	}
}

void AINSWeaponAttachment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSWeaponAttachment, WeaponOwner);
}

void AINSWeaponAttachment::CreateClientVisualAttachment()
{ 
	ClientVisualAttachment = GetWorld()->SpawnActorDeferred<AINSWeaponAttachment>(GetClass(), GetActorTransform(), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (GetClientVisualAttachment())
	{
		GetClientVisualAttachment()->bClientVisualAttachment = true;
		GetClientVisualAttachment()->FinishSpawning(ClientVisualAttachment->GetActorTransform());
	}
}

void AINSWeaponAttachment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AINSWeaponAttachment::ReceiveAttachmentEquipped(class AINSWeaponBase* WeaponEuippedBy)
{
	this->WeaponOwner = WeaponEuippedBy;
}

FORCEINLINE class USkeletalMeshComponent* AINSWeaponAttachment::GetAttachmentMeshComp() const
{
	return AttachmentMesh;
}

