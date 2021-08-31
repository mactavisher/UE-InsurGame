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

AINSWeaponAttachment_Optic::AINSWeaponAttachment_Optic(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
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
	TargetFOV = 70.f;
	HandIKXLocationValue = -5.f;
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
	if (WeaponOwner)
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
}

void AINSWeaponAttachment_Optic::AttachToWeaponSlot()
{
	Super::AttachToWeaponSlot();
	OpticMeshComp->AttachToComponent(AttachmentMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
}

void AINSWeaponAttachment_Optic::OnRep_OwnerWeapon()
{
	Super::OnRep_OwnerWeapon();
	if (WeaponOwner)
	{
		AINSCharacter* OwnerChar = WeaponOwner->GetOwnerCharacter();
		if (OwnerChar && OwnerChar->GetClass()->IsChildOf(AINSPlayerCharacter::StaticClass()))
		{
			AINSPlayerCharacter* PlayerChar = Cast<AINSPlayerCharacter>(OwnerChar);
			if (PlayerChar)
			{
				PlayerChar->Get1PAnimInstance()->SetAimHandIKXLocation(WeaponOwner->GetWeaponAimHandIKXLocation());
			}
		}
		const AINSPlayerStateBase* const PS = OwnerChar->GetPlayerState<AINSPlayerStateBase>();
		if ((PS && PS->IsABot()) || WeaponOwner->GetLocalRole() == ROLE_SimulatedProxy)
		{
			OpticMeshComp->DestroyComponent();
			SceneCaptureComp->DestroyComponent();
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
		return OpticMeshComp->GetSocketTransform(SightAlignerName);
	}
	return FTransform::Identity;
}
