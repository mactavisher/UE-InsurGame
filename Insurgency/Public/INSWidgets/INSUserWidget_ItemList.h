// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "INSWidgets/INSWidgetBase.h"
#include "INSUserWidget_ItemList.generated.h"

class UScrollBox;
class UINSUserWidget_Item;

/**
 *  this is a container widget which contains the items
 */
UCLASS()
class INSURGENCY_API UINSUserWidget_ItemList : public UINSWidgetBase
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	UScrollBox* ItemScrollBox;

	UPROPERTY()
	TArray<UINSUserWidget_Item*> ItemWidgets;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="ItemWidgetClass")
	TSubclassOf<UINSUserWidget_Item> ItemWidgetClass;
public:
	UFUNCTION(BlueprintCallable)
	virtual void SetItemScrollBox(UScrollBox* NewScrollBox);

	UFUNCTION(BlueprintCallable)
	virtual UScrollBox* GetScrollBox()const{return ItemScrollBox;}

	UFUNCTION(BlueprintCallable)
	virtual void CreateItemWidgets();

	virtual void RemoveAllItemWidgets();

	virtual void NativeConstruct() override;
	
};
