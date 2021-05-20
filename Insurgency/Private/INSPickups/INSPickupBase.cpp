// Fill out your copyright notice in the Description page of Project Settings.


#include "INSPickups/INSPickupBase.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "INSCharacter/INSPlayerCharacter.h"
#ifndef  AINSPlayerController
#include "INSCharacter/INSPlayerController.h"
#endif

DEFINE_LOG_CATEGORY(LogINSPickup);

AINSPickupBase::AINSPickupBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	SimpleCollisionComp = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("ColisionComp"));
	SimpleCollisionComp->SetBoxExtent(FVector(40.f, 3.f, 5.f));
	RootComponent = SimpleCollisionComp;
	InteractionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("InteractComp"));
	InteractionComp->SetSphereRadius(100.f);
	InteractionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionComp->SetupAttachment(RootComponent);
	SetReplicates(true);
	SetReplicateMovement(true);
	DisableTick();
	bAutoPickup = false;
	bAutoDestory = true;
}

void AINSPickupBase::BeginPlay()
{
	Super::BeginPlay();
}

void AINSPickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AINSPickupBase::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	if (NewOwner)
	{
		AINSPlayerController* const PlayerController = Cast<AINSPlayerController>(NewOwner);
		if (PlayerController)
		{
			UE_LOG(LogINSPickup, Log, TEXT("this pick up %s has set the owner for player %s and will give this to it"), *(GetName()), *(PlayerController->GetName()));
			GiveTo(PlayerController);
		}
	}
}


void AINSPickupBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (HasAuthority())
	{
		FRepMovement* const RepedMovement = (FRepMovement*)&GetReplicatedMovement();
		RepedMovement->LocationQuantizationLevel = EVectorQuantization::RoundOneDecimal;
		RepedMovement->VelocityQuantizationLevel = EVectorQuantization::RoundOneDecimal;
		RepedMovement->bRepPhysics = true;
		SetLifeSpan(120.f);
	}
	// query collision is no needed for dedicated server
	if (!IsNetMode(NM_DedicatedServer))
	{
		InteractionComp->OnComponentBeginOverlap.AddDynamic(this, &AINSPickupBase::HandleOnBeginOverlap);
		InteractionComp->OnComponentEndOverlap.AddDynamic(this, &AINSPickupBase::HandleOnEndOverlap);
	}
}

void AINSPickupBase::OnRep_ReplicatedMovement()
{
	//unpack the replicated movement data
	FRepMovement* RepedMovement = (FRepMovement*)&GetReplicatedMovement();
	RepedMovement->Location = RepedMovement->Location / 10.f;
	RepedMovement->LinearVelocity = RepedMovement->LinearVelocity / 10.f;
	Super::OnRep_ReplicatedMovement();
}

void AINSPickupBase::GatherCurrentMovement()
{
	Super::GatherCurrentMovement();
}

void AINSPickupBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSPickupBase, bIsActive);
}

void AINSPickupBase::OnRep_bIsActive()
{

}

void AINSPickupBase::HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogINSPickup, Log, TEXT("character %s entered this pick up"), *OtherActor->GetName());
	const AINSPlayerCharacter* const PlayerCharacter = Cast<AINSPlayerCharacter>(OtherActor);
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		AINSPlayerController* const PlayerController = Cast<AINSPlayerController>(PlayerCharacter->GetController());
		if (PlayerController)
		{
			PlayerController->ReceiveEnterPickups(this);
		}
	}
}

void AINSPickupBase::HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogINSPickup, Log, TEXT("%s leaved this pick up"), *OtherActor->GetName());
	const AINSPlayerCharacter* const PlayerCharacter = Cast<AINSPlayerCharacter>(OtherActor);
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		AINSPlayerController* const PlayerController = Cast<AINSPlayerController>(PlayerCharacter->GetController());
		if (PlayerController)
		{
			PlayerController->ReceiveLeavePickups(this);
		}
	}
}

void AINSPickupBase::ReceiveInteractFinished(class AController* Player)
{
	SetOwner(Player);
	if (GetOwner())
	{
		bIsActive = false;
		if (HasAuthority())
		{
			OnRep_bIsActive();
		}
	}
}

void AINSPickupBase::EnableTick()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
}

void AINSPickupBase::DisableTick()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.SetTickFunctionEnable(false);
}

void AINSPickupBase::GiveTo(class AController* PlayerToGive)
{

}

void AINSPickupBase::OnRep_Owner()
{
	Super::OnRep_Owner();
}

