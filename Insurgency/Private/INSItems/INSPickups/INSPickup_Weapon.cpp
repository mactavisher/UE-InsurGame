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
	
}

void AINSPickup_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}


void AINSPickup_Weapon::GiveTo(class AController* PlayerToGive)
{
  Super::GiveTo(PlayerToGive);
}


void AINSPickup_Weapon::BeginPlay()
{
	Super::BeginPlay();
}


void AINSPickup_Weapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AINSPickup_Weapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

