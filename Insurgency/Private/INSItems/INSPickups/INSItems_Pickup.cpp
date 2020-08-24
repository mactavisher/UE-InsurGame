// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSPickups/INSItems_Pickup.h"
#include "Components/SphereComponent.h"
#ifndef AINSPlayerCharacter
#include "Insurgency/Public/INSCharacter/INSPlayerCharacter.h"
#endif
AINSItems_Pickup::AINSItems_Pickup(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bAutoPickup = false;
	InteractionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("InteractionComp"));
	InteractionComp->InitSphereRadius(100.f);
	InteractionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionComp->SetCollisionResponseToAllChannels(ECR_Overlap);
	//InteractionComp->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void AINSItems_Pickup::HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AINSItems_Pickup::HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AINSItems_Pickup::GiveThisToPlayer(class AController* NewClaimedPlayer)
{

}

void AINSItems_Pickup::BeginPlay()
{
	InteractionComp->OnComponentBeginOverlap.AddDynamic(this, &AINSItems_Pickup::HandleOnBeginOverlap);
	InteractionComp->OnComponentEndOverlap.AddDynamic(this, &AINSItems_Pickup::HandleOnEndOverlap);
}
