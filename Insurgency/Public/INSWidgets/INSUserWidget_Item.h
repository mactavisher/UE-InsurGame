// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSWidgetBase.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "INSUserWidget_Item.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSUserWidget_Item : public UINSWidgetBase
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="ItemId")
	int32 ItemId;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Button")
	UButton* Button;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Button")
	UImage* ItemImage;

	UPROPERTY()
	UTexture* ImageTexture;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="ItemName")
	FString ItemName;
	
	FSlateBrush* ImageSlateBrush;

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable)
	virtual void SetButton(UButton* InButton);
	
	UFUNCTION(BlueprintCallable)
	virtual void SetItemName(FString NewItemName){ItemName = NewItemName;}
	
	UFUNCTION(BlueprintCallable)
	virtual void SetItemImage(UImage* InImage){ItemImage = InImage;}
	
	UFUNCTION(BlueprintCallable)
	virtual void SetItemId(int32 NewItemId){ItemId = NewItemId;}
	
	UFUNCTION(BlueprintCallable)
	virtual FSlateBrush GetImageSlateBrush();
};
