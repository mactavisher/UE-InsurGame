// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSWidgets/INSWidgetBase.h"
#include "INSWidget_CrossHair_Dot.generated.h"


class UTexture2D;
/**
 *
 */
UCLASS()
class INSURGENCY_API UINSWidget_CrossHair_Dot : public UINSWidgetBase
{
	GENERATED_UCLASS_BODY()
protected:

	/** Current Cross Hair tint Color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Color")
		FLinearColor CurrentCrossHairTintColor;

	/** Default Cross Hair tint Color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Color")
		FLinearColor DefaultCrossHairTintColor;

	/** after this time will reset tint color to default */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponCrossHair|Texture")
		float CrossHairColorResetTime;

	/** dot cross hair texture */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Texture")
		UTexture2D* DotTexture;

	/** tint color reset timer */
	UPROPERTY()
		FTimerHandle CrossHairTintColorResetHandle;
public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CrossHair|Color")
		virtual FLinearColor GetCrossHairTintColor() { return CurrentCrossHairTintColor; }

	UFUNCTION(BlueprintCallable, Category = "CrossHair|Color")
		virtual void SetCrossHairTintColor(FLinearColor NewTintColor);

	UFUNCTION()
		virtual void ResetCrossHairTint() { SetCrossHairTintColor(DefaultCrossHairTintColor); };
};
