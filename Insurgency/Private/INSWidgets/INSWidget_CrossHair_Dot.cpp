// Fill out your copyright notice in the Description page of Project Settings.


#include "INSWidgets/INSWidget_CrossHair_Dot.h"
#include "INSCharacter/INSPlayerController.h"
#include "TimerManager.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "UMG\Public\Blueprint\WidgetBlueprintLibrary.h"

UINSWidget_CrossHair_Dot::UINSWidget_CrossHair_Dot(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	DefaultCrossHairTintColor = FLinearColor::White;
	CurrentCrossHairTintColor = DefaultCrossHairTintColor;
	CrossHairColorResetTime = 0.4f;
	bShowWidget = true;
}

void UINSWidget_CrossHair_Dot::SetCrossHairTintColor(FLinearColor NewTintColor)
{
	CurrentCrossHairTintColor = NewTintColor;
	//if timer already set ,just reset it ,no need to check timer state here
	GetOwningINSPlayer()->GetWorldTimerManager().SetTimer(CrossHairTintColorResetHandle, this, &UINSWidget_CrossHair_Dot::ResetCrossHairTint, 1.0f, false, CrossHairColorResetTime);
}

