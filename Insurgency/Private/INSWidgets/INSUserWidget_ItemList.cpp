// Fill out your copyright notice in the Description page of Project Settings.


#include "INSWidgets/INSUserWidget_ItemList.h"

#include "INSCore/INSGameInstance.h"
#include "INSWidgets/INSUserWidget_Item.h"

void UINSUserWidget_ItemList::SetItemScrollBox(UScrollBox* NewScrollBox)
{
	ItemScrollBox = NewScrollBox;
}

void UINSUserWidget_ItemList::CreateItemWidgets()
{
	const UINSGameInstance* const OwingGameInstance = GetGameInstance<UINSGameInstance>();
	if (!OwingGameInstance)
	{
		return;
	}
	TArray<FWeaponInfoData> WeaponInfos;
	OwingGameInstance->GetItemManager()->GetAllWeaponsInfos(WeaponInfos);
	if (WeaponInfos.Num() > 0)
	{
		for (FWeaponInfoData WeaponInfo : WeaponInfos)
		{
			if (ItemWidgetClass)
			{
				UINSUserWidget_Item* ItemWidget = CreateWidget<UINSUserWidget_Item>(GetOwningLobbyPlayerController(), ItemWidgetClass, FName("WeaponItemWidget"));
				ItemWidget->SetItemName(WeaponInfo.ItemName.ToString());
				ItemWidget->SetItemId(WeaponInfo.ItemId);
			}
		}
	}
	if (ItemScrollBox && ItemWidgets.Num() > 0)
	{
		for (UINSUserWidget_Item* const ItemWidget : ItemWidgets)
		{
			ItemScrollBox->AddChild(ItemWidget);
		}
	}
}

void UINSUserWidget_ItemList::RemoveAllItemWidgets()
{
	if (ItemScrollBox && ItemWidgets.Num() > 0)
	{
		for (UINSUserWidget_Item* const ItemWidget : ItemWidgets)
		{
			ItemWidget->RemoveFromParent();
		}
		ItemWidgets.Empty(0);
	}
}

void UINSUserWidget_ItemList::NativeConstruct()
{
	Super::NativeConstruct();
}
