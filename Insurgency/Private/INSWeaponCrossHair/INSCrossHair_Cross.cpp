// Fill out your copyright notice in the Description page of Project Settings.


#include "INSWeaponCrossHair/INSCrossHair_Cross.h"

#include "CanvasItem.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Engine/Canvas.h"

UINSCrossHair_Cross::UINSCrossHair_Cross(const FObjectInitializer& ObjectInitalizer) : Super(ObjectInitalizer)
{
	CrossHairCurrentTintColor = WeaponCrossHairInfo.CrossHairDefaultTintColor;
}

void UINSCrossHair_Cross::DrawCrossHair(class UCanvas* InCanvas, class AINSWeaponBase* InWeapon, FLinearColor DrawColor)
{
	const float ScaleValue = 2.f;
	const FVector2D CanvasScale = GetCanvasScale(InCanvas);
	const float SpreadModifier = InWeapon->GetWeaponCurrentSpread();
	const float BiasX = WeaponCrossHairInfo.CenterRadius * SpreadModifier * CanvasScale.X * ScaleValue;
	const float BiasY = WeaponCrossHairInfo.CenterRadius * SpreadModifier * CanvasScale.Y * ScaleValue;
	const float LeftLineCoordX = InCanvas->SizeX / 2.f - BiasX;
	const float LeftLineEndCoordX = LeftLineCoordX - WeaponCrossHairInfo.LineLength * CanvasScale.X;
	const float LeftLineCoordY = InCanvas->SizeY / 2.f;
	const float RightLineCoordX = InCanvas->SizeX / 2.f + BiasX;
	const float RightLineEndCoordX = RightLineCoordX + WeaponCrossHairInfo.LineLength * CanvasScale.X;
	const float RightLineCoordY = InCanvas->SizeY / 2.f;
	const float UpLineCoordX = InCanvas->SizeX / 2.f;
	const float UpperLineCoordY = InCanvas->SizeY / 2.f - BiasY;
	const float UpperLineEndCoordY = UpperLineCoordY - WeaponCrossHairInfo.LineLength * CanvasScale.Y;
	const float DownLineCoordX = InCanvas->SizeX / 2.f;
	const float DownLneCoordY = InCanvas->SizeY / 2.f + BiasY;
	const float DownLineEndCoordY = DownLneCoordY + WeaponCrossHairInfo.LineLength * CanvasScale.Y;
	//draw CrossHair left part
	FCanvasLineItem LeftLineItem(FVector2D(LeftLineCoordX, LeftLineCoordY), FVector2D(LeftLineEndCoordX, LeftLineCoordY));
	LeftLineItem.SetColor(CrossHairCurrentTintColor);
	LeftLineItem.LineThickness = WeaponCrossHairInfo.LineScale;
	InCanvas->DrawItem(LeftLineItem);

	//draw CrossHair right part
	FCanvasLineItem RightLineItem(FVector2D(RightLineCoordX, RightLineCoordY), FVector2D(RightLineEndCoordX, LeftLineCoordY));
	RightLineItem.SetColor(CrossHairCurrentTintColor);
	RightLineItem.LineThickness = WeaponCrossHairInfo.LineScale;
	InCanvas->DrawItem(RightLineItem);

	//draw CrossHair up part
	FCanvasLineItem UpLineItem(FVector2D(UpLineCoordX, UpperLineCoordY), FVector2D(UpLineCoordX, UpperLineEndCoordY));
	UpLineItem.SetColor(CrossHairCurrentTintColor);
	UpLineItem.LineThickness = WeaponCrossHairInfo.LineScale;
	InCanvas->DrawItem(UpLineItem);

	//draw CrossHair down part
	FCanvasLineItem DownLineItem(FVector2D(DownLineCoordX, DownLneCoordY), FVector2D(DownLineCoordX, DownLineEndCoordY));
	DownLineItem.SetColor(CrossHairCurrentTintColor);
	DownLineItem.LineThickness = WeaponCrossHairInfo.LineScale;
	InCanvas->DrawItem(DownLineItem);
}
