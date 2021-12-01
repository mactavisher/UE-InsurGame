// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCore/INSGameInstance.h"
#include "INSCore/INSItemManager.h"
UINSGameInstance::UINSGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ItemManagerClass = UINSItemManager::StaticClass();
}

void UINSGameInstance::Init()
{
	Super::Init();
	if(ItemManagerClass)
	{
		ItemManager = NewObject<UINSItemManager>(this,ItemManagerClass,TEXT("ItemManager"));
	}
	if(ItemManager)
	{
		ItemManager->SetOwingGameInstance(this);
	}
}
