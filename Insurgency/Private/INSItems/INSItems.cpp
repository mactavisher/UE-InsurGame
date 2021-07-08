// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSItems.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSCharacter/INSPlayerController.h"
#include "Engine/Texture2D.h"
#include "Components/SphereComponent.h"
#include "Insurgency/Public/INSCharacter/INSPlayerCharacter.h"

// Sets default values
AINSItems::AINSItems(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bIsActive = true;
	bIsWeapon = false;
	ItemType = EItemType::NONE;
	InteractTime = 2.f;
	bReplicates = true;
	SetReplicatingMovement(true);
}

// Called when the game starts or when spawned
void AINSItems::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void AINSItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AINSItems::NotifyCharacterEnter(class AINSPlayerCharacter* CharacterToNotify)
{
	//hook but do nothing by default
}

void AINSItems::NotifyCharacterLeave(class AINSPlayerCharacter* CharacterToNotify)
{
	//hook but do nothing by default
}

void AINSItems::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AINSItems::OnRep_bIsActive()
{
	//hook but do nothing by default
}

void AINSItems::HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//hook but do nothing by default
}

void AINSItems::HandleOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//hook but do nothing by default
}

void AINSItems::ReceiveDetectedBy(class AController* PlayerInstigator, class ACharacter* DetectPlayerCharacter)
{
	//hook but do nothing by default
}

void AINSItems::ReceiveInteractFinished(class AController* Player)
{
	//hook but do nothing by default
}

void AINSItems::ShowItemIcon(class AController* PlayerInstigator, class ACharacter* DetectPlayerCharacter)
{
	//hook but do nothing by default
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

void AINSItems::OnRep_Owner()
{
	Super::OnRep_Owner();
}

