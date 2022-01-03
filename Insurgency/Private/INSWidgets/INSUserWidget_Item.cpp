// Fill out your copyright notice in the Description page of Project Settings.


#include "INSWidgets/INSUserWidget_Item.h"
#include "UMG/Public/Components/Button.h"
#include "SlateCore/Public/Styling/SlateBrush.h"

void UINSUserWidget_Item::NativeConstruct()
{
	Super::NativeConstruct();
}

void UINSUserWidget_Item::SetButton(UButton* InButton)
{
}

FSlateBrush UINSUserWidget_Item::GetImageSlateBrush()
{
	if (ImageSlateBrush)
	{
		return *ImageSlateBrush;
	}
	FSlateBrush SlateBrush;
	SlateBrush.DrawAs = ESlateBrushDrawType::Image;
	SlateBrush.SetResourceObject(ImageTexture);
	ImageSlateBrush = &SlateBrush;
	return *ImageSlateBrush;
}
