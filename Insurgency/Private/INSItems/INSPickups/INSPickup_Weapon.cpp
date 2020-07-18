// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSPickups/INSPickup_Weapon.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "INSCharacter/INSPlayerController.h"

AINSPickup_Weapon::AINSPickup_Weapon(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	DestroyTime = 20.f;
	SetReplicates(true);
	SetReplicateMovement(true);
	VisualMeshComp = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("VisualMeshComp"));
	RootComponent = VisualMeshComp;
	VisualMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AINSPickup_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME_CONDITION(AINSPickup_Weapon, ActualWeaponClass, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AINSPickup_Weapon, VisualMesh, COND_InitialOnly);
}

void AINSPickup_Weapon::HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
// 	if (OtherActor&&OtherActor->GetClass()->IsChildOf(AINSCharacter::StaticClass()))
// 	{
// 		
// 	}
}

void AINSPickup_Weapon::HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

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

void AINSPickup_Weapon::SetViualMesh(USkeletalMesh* NewVisualMesh)
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

void AINSPickup_Weapon::OnRep_VisualMesh()
{
	VisualMeshComp->SetSkeletalMesh(VisualMesh);
}

void AINSPickup_Weapon::DestroyThisWeaponPickup()
{
	Destroy(false, true);
}
