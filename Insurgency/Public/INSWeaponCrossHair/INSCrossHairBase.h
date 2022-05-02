// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "INSCrossHairBase.generated.h"

class APlayerController;
class UCanvas;

/**
 *  Base class for weapon cross hairs,sub CrossHair types must override DrawCrossHair
 *  To get get work
 */
UCLASS(abstract, notplaceable, NotBlueprintable)
class INSURGENCY_API UINSCrossHairBase : public UObject
{
	GENERATED_UCLASS_BODY()
	/** Owner Player */
	UPROPERTY()
	APlayerController* OwnerPlayer;

	UPROPERTY()
	AINSWeaponBase* OwnerWeapon;

	/** draw color of this CrossHair */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CrossHair")
	FLinearColor CrossHairCurrentTintColor;

	/** base canvas size ,we design everything against 1080p */
	UPROPERTY()
	FVector2D BaseSize;

	/**
	 * @Desc   calculate the canvas scale against base size
	 * @return canvas scale Fvector2D
	 */
	virtual FVector2D GetCanvasScale(const class UCanvas* InCanvas);

public:
	/**
	 * @Desc set the player owner of this cross hair
	 * @Param NewOwnerPlayer New owner to set for this cross hair
	 */
	virtual void SetOwnerPlayer(APlayerController* NewOwnerPlayer);

	/**
	 * @Desc set the weapon owner of this cross hair
	 * @Param NewWeaponOwner New weapon owner to set for this cross hair
	 */
	virtual void SetOwnerWeapon(AINSWeaponBase* NewWeaponOwner);

	/**
	 * @Desc draw canvas based cross hair and called by HUD DrawHud Main loop
	 * @Param InCanvas  target canvas to draw this cross hair on
	 * @Param InWeapon weapon this cross hair belongs to
	 * @Param DrawColor cross hair color to draw
	 */
	virtual void DrawCrossHair(class UCanvas* InCanvas, class AINSWeaponBase* InWeapon, FLinearColor DrawColor);
};
