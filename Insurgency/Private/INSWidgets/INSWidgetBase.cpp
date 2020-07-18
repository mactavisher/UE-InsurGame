// Fill out your copyright notice in the Description page of Project Settings.


#include "INSWidgets/INSWidgetBase.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSCharacter/INSPlayerController.h"

UINSWidgetBase::UINSWidgetBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bShowWidget = true;
}

void UINSWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

class AINSPlayerController* UINSWidgetBase::GetOwningINSPlayer()
{
	return GetOwningPlayer<AINSPlayerController>();
}
