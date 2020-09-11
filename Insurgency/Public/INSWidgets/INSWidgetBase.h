// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "INSWidgetBase.generated.h"

class AINSWeaponBase;
class AINSPlayerController;
class AINSCharacter;
/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSWidgetBase : public UUserWidget
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VisibilityControl")
		uint8 bShowWidget : 1;
	   

	TWeakObjectPtr<AINSWeaponBase> RefrencedWeapon;

	virtual void SetRefWeapon(AINSWeaponBase* WeaponRef);

	virtual class AINSWeaponBase* GetRefWeapon()const { return RefrencedWeapon.Get(); }

	virtual void NativeConstruct()override;

	virtual class  AINSPlayerController* GetOwningINSPlayer();

};
