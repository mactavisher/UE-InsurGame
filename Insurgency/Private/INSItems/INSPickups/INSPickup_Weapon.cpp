// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSPickups/INSPickup_Weapon.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "INSCharacter/INSPlayerController.h"
#ifndef AINSPlayerCharacter
#include "Insurgency/Public/INSCharacter/INSPlayerCharacter.h"
#endif
#ifndef USphereComponent
#include "Components/SphereComponent.h"
#endif

AINSPickup_Weapon::AINSPickup_Weapon(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	DestroyTime = 20.f;
	SetReplicates(true);
	SetReplicateMovement(true);
	VisualMeshComp = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("VisualMeshComp"));
	RootComponent = VisualMeshComp;
	VisualMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionComp->SetupAttachment(RootComponent);
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	InteractionComp->SetHiddenInGame(false);
#endif
}

void AINSPickup_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AINSPickup_Weapon, VisualMesh, COND_InitialOnly);
}

void AINSPickup_Weapon::HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AINSPlayerCharacter* const JustOverLappedPlayerCharacter = Cast<AINSPlayerCharacter>(OtherActor);
	if (JustOverLappedPlayerCharacter)
	{
		NotifyCharacterEnter(JustOverLappedPlayerCharacter);
	}
}

void AINSPickup_Weapon::HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AINSPlayerCharacter* const JustEndOverlappedPlayerCharacter = Cast<AINSPlayerCharacter>(OtherActor);
	if (JustEndOverlappedPlayerCharacter)
	{
		NotifyCharacterLeave(JustEndOverlappedPlayerCharacter);
	}
}

void AINSPickup_Weapon::GiveThisToPlayer(class AController* NewClaimedPlayer)
{
	if (GetClaimedPlayer())
	{
		UE_LOG(LogTemp, Log, TEXT("this pick up has claimed player"));
		return;
	}
	if (ClaimedPlayer->GetClass()->IsChildOf(AINSPlayerController::StaticClass()))
	{
		AINSPlayerController* PlayerController = CastChecked<AINSPlayerController>(ClaimedPlayer);
	}
}

void AINSPickup_Weapon::NotifyCharacterEnter(class AINSPlayerCharacter* CharacterToNotify)
{
	Super::NotifyCharacterEnter(CharacterToNotify);
	if (CharacterToNotify)
	{
		if (!CharacterToNotify->GetIsCharacterDead())
		{
			AINSPlayerController* PC = Cast<AINSPlayerController>(CharacterToNotify->GetController());
			if (PC)
			{
				PC->ReceiveEnterPickups(this);
			}
		}
	}
}

void AINSPickup_Weapon::NotifyCharacterLeave(class AINSPlayerCharacter* CharacterToNotify)
{
	Super::NotifyCharacterLeave(CharacterToNotify);
	if (CharacterToNotify)
	{
		AINSPlayerController* PC = Cast<AINSPlayerController>(CharacterToNotify->GetController());
		if (PC)
		{
			PC->ReceiveLeavePickups(this);
		}
	}
}

void AINSPickup_Weapon::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(DestoryTimer, this, &AINSPickup_Weapon::DestroyThisWeaponPickup, 1.f, false, DestroyTime);
		VisualMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		VisualMeshComp->SetSimulatePhysics(true);
		if (VisualMeshComp&&VisualMeshComp->Mobility == EComponentMobility::Movable)
		{
			VisualMeshComp->AddImpulseAtLocation(GetActorForwardVector()*10.f+FVector(500.f,200.f,500.f),GetActorLocation());
		}
	}
}

AController* AINSPickup_Weapon::GetClaimedPlayer()
{
	return  ClaimedPlayer.Get();
}

void AINSPickup_Weapon::SetViualMesh(class USkeletalMesh* NewVisualMesh)
{
	VisualMesh = NewVisualMesh;
}

void AINSPickup_Weapon::PostNetReceiveLocationAndRotation()
{
	Super::PostNetReceiveLocationAndRotation();
	SetActorLocationAndRotation(GetReplicatedMovement().Location, GetReplicatedMovement().Rotation);
}

void AINSPickup_Weapon::GatherCurrentMovement()
{
	Super::GatherCurrentMovement();
	FRepMovement OriginReplicatedMovement = GetReplicatedMovement();
	FRepMovement OptimizedReplicatedMovement;
	OptimizedReplicatedMovement.RotationQuantizationLevel = ERotatorQuantization::ByteComponents;
	OptimizedReplicatedMovement.LocationQuantizationLevel = EVectorQuantization::RoundWholeNumber;
	OptimizedReplicatedMovement.VelocityQuantizationLevel = EVectorQuantization::RoundOneDecimal;
	OptimizedReplicatedMovement.Location = FVector_NetQuantize100(OriginReplicatedMovement.Location);
	OptimizedReplicatedMovement.Rotation = OriginReplicatedMovement.Rotation;
	OptimizedReplicatedMovement.bRepPhysics = false;
	SetReplicatedMovement(OptimizedReplicatedMovement);
}

void AINSPickup_Weapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	VisualMeshComp->GetBodyInstance()->SetCollisionProfileName(TEXT("PickupAble"));
	InteractionComp->GetBodyInstance()->SetCollisionProfileName(TEXT("Trigger"));
}

void AINSPickup_Weapon::OnRep_VisualMesh()
{
	VisualMeshComp->SetSkeletalMesh(VisualMesh);
}

void AINSPickup_Weapon::DestroyThisWeaponPickup()
{
	Destroy(false, true);
}
