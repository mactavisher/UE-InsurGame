// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSWeaponAttachments/INSWeaponAttachment_Optic.h"
#include "Kismet/GameplayStatics.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "Components/SceneCaptureComponent2D.h"
#include "INSAnimation/INSFPAnimInstance.h"
#ifndef AINSWeaponBase
#include "INSItems/INSWeapons/INSWeaponBase.h"
#endif

AINSWeaponAttachment_Optic::AINSWeaponAttachment_Optic(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	OpticMeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("OpticMesh"));
	SceneCaptureComp = ObjectInitializer.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("SceneCaptureComp"));
	SceneCaptureComp->SetupAttachment(RootComponent);
	//SceneCaptureComp->SetActive(false);
	OpticMeshComp->SetHiddenInGame(true);
	OpticMeshComp->AlwaysLoadOnClient = true;
	CompatibleWeaponSlots.Add(EWeaponAttachmentType::SIGHT);
	AttachmentType = EWeaponAttachmentType::SIGHT;
	OpticMeshComp->SetupAttachment(RootComponent);
	OpticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OpticMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TargetFOV = 70.f;
	BaseHandIKXLocationValue = -4.f;
	bEnableDualRenderOptic = false;
}

float AINSWeaponAttachment_Optic::GetADSAlpha()
{
	if (WeaponOwner)
	{
		return WeaponOwner->GetWeaponADSAlpha();
	}
	return 0.f;
}

void AINSWeaponAttachment_Optic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	//only we have a weapon owner and player controlled client need to update this
	if (WeaponOwner && !WeaponOwner->GetIsClientCosmeticWeapon())
	{
		const AController* const PC = Cast<AController>(WeaponOwner->GetOwner());
		if (PC)
		{
			const APlayerState* PS = PC->PlayerState;
			if (PS && !PS->IsABot())
			{
				if (GetADSAlpha() > 0.65f)
				{
					AttachmentMeshComp->SetHiddenInGame(true);
					OpticMeshComp->SetHiddenInGame(false);
					SceneCaptureComp->PrimaryComponentTick.bCanEverTick = true;
					SceneCaptureComp->PrimaryComponentTick.SetTickFunctionEnable(true);
				}
				else
				{
					AttachmentMeshComp->SetHiddenInGame(false);
					OpticMeshComp->SetHiddenInGame(true);
					SceneCaptureComp->PrimaryComponentTick.bCanEverTick = false;
					SceneCaptureComp->PrimaryComponentTick.SetTickFunctionEnable(false);
				}
			}
		}
	}
}

void AINSWeaponAttachment_Optic::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	OpticMeshComp->AttachToComponent(AttachmentMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
	SceneCaptureComp->AttachToComponent(OpticMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
	SceneCaptureComp->AddRelativeLocation(FVector(50.f, 0.f, 0.f));
	if (bEnableDualRenderOptic)
	{
		SceneCaptureComp->PrimaryComponentTick.bCanEverTick = true;
		SceneCaptureComp->PrimaryComponentTick.SetTickFunctionEnable(true);
	}
	else
	{
		SceneCaptureComp->PrimaryComponentTick.bCanEverTick = false;
		SceneCaptureComp->PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}

void AINSWeaponAttachment_Optic::AttachToWeaponSlot()
{
	Super::AttachToWeaponSlot();
	if (WeaponOwner && WeaponOwner->GetRequireExtraOpticRail())
	{
		RootComponent->AttachToComponent(WeaponOwner->GetOpticRailComp(), FAttachmentTransformRules::SnapToTargetIncludingScale,TEXT("Optic"));
	}
}

void AINSWeaponAttachment_Optic::OnRep_OwnerWeapon()
{
	Super::OnRep_OwnerWeapon();
	if (WeaponOwner)
	{
		AINSCharacter* OwnerChar = WeaponOwner->GetOwnerCharacter();
		if (OwnerChar)
		{
			OwnerChar->SetAimHandsXLocation(WeaponOwner->GetWeaponAimHandIKXLocation());
			const AINSPlayerStateBase* const PS = OwnerChar->GetPlayerState<AINSPlayerStateBase>();
			if ((PS && PS->IsABot()) || WeaponOwner->GetLocalRole() == ROLE_SimulatedProxy)
			{
				OpticMeshComp->DestroyComponent();
				SceneCaptureComp->DestroyComponent();
			}
		}
	}
}

void AINSWeaponAttachment_Optic::BeginPlay()
{
	Super::BeginPlay();
}

FTransform AINSWeaponAttachment_Optic::GetOpticSightTransform()
{
	const FName SightAlignerName = FName(TEXT("SightAligner"));
	if (OpticMeshComp->DoesSocketExist(SightAlignerName))
	{
		return OpticMeshComp->GetSocketTransform(SightAlignerName, RTS_World);
	}
	return FTransform::Identity;
}
