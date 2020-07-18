// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSItems.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSCharacter/INSPlayerController.h"
#include "Engine/Texture2D.h"
#include "Components/SphereComponent.h"

// Sets default values
AINSItems::AINSItems(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	DisableTick();
	InteractCollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("InteractComp"));
	bIsActive = true;
	bIsWeapon = false;
	ItemType = EItemType::NONE;
	InteractTime = 2.f;
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AINSItems::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority)
	{
		InteractCollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		InteractCollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		InteractCollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		InteractCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AINSItems::HandleOnBeginOverlap);
		InteractCollisionComp->OnComponentEndOverlap.AddDynamic(this, &AINSItems::HandleOnEndOverlap);
	}
	else
	{
		//clients no need to enable collision
		InteractCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Called every frame
void AINSItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AINSItems::NotifyCharacterEnterIventoryItem(class AINSPlayerCharacter* CharacterToNotify)
{

}

void AINSItems::NotifyCharacterLeaveInventoryItem(class AINSPlayerCharacter* CharacterToNotify)
{

}

void AINSItems::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AINSItems::OnRep_bIsActive()
{

}

void AINSItems::HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void AINSItems::HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AINSItems::ReceiveDetectedBy(class AController* PlayerInstigator, class ACharacter* DetectPlayerCharacter)
{

}

void AINSItems::ReceiveInteractFinished(class AController* Player)
{

}

void AINSItems::ShowItemIcon(class AController* PlayerInstigator, class ACharacter* DetectPlayerCharacter)
{

}

void AINSItems::EnableTick()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
}

void AINSItems::DisableTick()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.SetTickFunctionEnable(false);
}

AINSPlayerController* AINSItems::GetOwnerPlayer()
{
	return CastChecked<AINSPlayerController>(GetOwner());
}

