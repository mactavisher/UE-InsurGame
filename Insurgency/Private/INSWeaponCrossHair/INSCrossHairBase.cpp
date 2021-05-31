// Fill out your copyright notice in the Description page of Project Settings.


#include "INSWeaponCrossHair/INSCrossHairBase.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSHud/INSHUDBase.h"
#include "Engine/Canvas.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

UINSCrossHairBase::UINSCrossHairBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	BaseSize = FVector2D(1920.f, 1080.f);
}

FVector2D UINSCrossHairBase::GetCanvasScale(const class UCanvas* InCanvas)
{
	const float ScaleX = InCanvas->SizeX / BaseSize.X;
	const float ScaleY = InCanvas->SizeY / BaseSize.Y;
	return FVector2D(ScaleX, ScaleY);
}

void UINSCrossHairBase::SetOwner(APlayerController* NewOwner)
{
	Owner = NewOwner;
}

void UINSCrossHairBase::DrawCrossHair(class UCanvas* InCanvas, class AINSWeaponBase* InWeapon, FLinearColor DrawColor)
{

}
