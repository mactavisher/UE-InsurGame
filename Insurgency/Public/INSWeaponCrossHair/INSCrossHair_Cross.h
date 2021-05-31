// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSWeaponCrossHair/INSCrossHairBase.h"
#include "INSCrossHair_Cross.generated.h"


USTRUCT(BlueprintType)
struct FWeaponCrossHairInfo
{
	GENERATED_USTRUCT_BODY()

		/** default cross hair tint color */
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CrossHair")
		FLinearColor CrossHairDefaultTintColor;

	/** extra effect tint color ,for example , when contact with enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CrossHair")
		FLinearColor CrossHairThreatenTintColor;

	/** Gap between each cross hair part */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHair")
		float CenterRadius;

	/** Gap between each cross hair part */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHair")
		float LineScale;

	/** Gap between each cross hair part */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHair")
		float LineLength;

	FWeaponCrossHairInfo()
		: CrossHairDefaultTintColor(FLinearColor::White)
		, CrossHairThreatenTintColor(FLinearColor::Red)
		, CenterRadius(6.f)
		, LineScale(1.f)
		, LineLength(8.f)
	{
	}
};


/**
 * cross type weapon cross hair
 */
UCLASS(BlueprintType, Blueprintable, notplaceable)
class INSURGENCY_API UINSCrossHair_Cross : public UINSCrossHairBase
{
	GENERATED_UCLASS_BODY()

	/** CrossHair Config data */
	UPROPERTY()
		FWeaponCrossHairInfo WeaponCrossHairInfo;

public:
	//~ Begin UINSCrossHairBase Interface
	virtual void DrawCrossHair(class UCanvas* InCanvas, class AINSWeaponBase* InWeapon, FLinearColor DrawColor)override;
	//~ End UINSCrossHairBase Interface

};
