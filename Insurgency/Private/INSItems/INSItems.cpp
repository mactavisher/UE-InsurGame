// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSItems.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSCharacter/INSPlayerController.h"
#include "Engine/Texture2D.h"
#include "Components/SphereComponent.h"
#include "Insurgency/Public/INSCharacter/INSPlayerCharacter.h"
#include "INSComponents/INSInventoryComponent.h"

// Sets default values
AINSItems::AINSItems(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ItemType = EItemType::NONE;
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

void AINSItems::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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

void AINSItems::InitItemInfoByInventorySlot(const FInventorySlot& InventorySlot)
{
}

void AINSItems::SetItemInfo(FItemInfoData& NewItemInfoData)
{
	ItemInfoData = NewItemInfoData;
}
