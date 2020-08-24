// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "INSHUDBase.generated.h"

class UINSWidget_CrossHair_Dot;
class UINSWidgetBase;
class AINSPlayerController;
class AINSWeaponBase;

/**
 *  In Game HUD Class
 */
UCLASS()
class INSURGENCY_API AINSHUDBase : public AHUD
{
	GENERATED_UCLASS_BODY()

	/** a simple center dot texture drawn on screen as cross hair */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<UINSWidget_CrossHair_Dot> DotCrossHairWidgetClass;

	/** stores all the Widget instances that Managed by this HUD for later access purposes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widgets")
		TArray<UINSWidgetBase*> WidgetInstances;

	/** dot cross hair widget instance */
	UPROPERTY()
		UINSWidget_CrossHair_Dot* DotCrossHairWidgetPtr;

	/** each part of cross hair length when drawing cross hair without using widget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CrossHair")
		float CrossHairLineLength;

	/** indicates whether or not to use HUD to draw linear color line as cross hair*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CrossHair")
		uint8 bUsingHudCrossHair : 1;

	/** indicate if should update cross hair draw call this frame */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHair")
		uint8 bShowCrossHair : 1;

	/** standard screen size X which default value is based on 1080p(1920*1080) to calculate Hud item scale*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHair")
		float StandardSizeX;

	/** standard screen size Y which default value is based on 1080p(1920*1080) to calculate Hud item scale*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHair")
		float StandardSizeY;

	/** Gap between each cross hair part */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHair")
		float CenterRadius;

	/** default cross hair tint color */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CrossHair")
		FLinearColor CrossHairCurrentTintColor;

	/** default cross hair tint color */
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="CrossHair")
	    FLinearColor CrossHairDefaultTintColor;

	/** extra effect tint color ,for example , when contact with enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CrossHair")
		FLinearColor CrossHairThreatenTintColor;

	/** extra effect tint color ,for example , when contact with enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
	uint8 bShowItemInfo:1;

	/** extra effect tint color ,for example , when contact with enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
	UTexture2D* ItemTexture;

	/** current weapon the player equipped */
	TWeakObjectPtr<AINSWeaponBase> CurrentWeapon;

	/** last actor we see */
	TWeakObjectPtr<AActor> LastSeenActor;
	

protected:
	/************************************************************************/
	/* delegates                                                            */
	/************************************************************************/
	FScriptDelegate WeaponAimDelegate;

	FScriptDelegate WeaponStopAimDelegate;

	FScriptDelegate WeaponReloadDelegate;

	FScriptDelegate WeaponFinishReloadDelegate;

protected:
	FTimerHandle Timer_ResetCrossHairTintColor;

protected:

	/** called when game starts */
	virtual void BeginPlay()override;

	/** draw HUD loop */
	virtual void DrawHUD()override;

	virtual void DrawMyTeamInfo();

	/** create widgets needed for this game modes */
	virtual void CreateWidgetInstances();

	/** bind weapon delegate which meed to update screen drawing items */
	virtual void BindDelegate();

	/** remove all weapon delegate already binded */
	virtual void RemoveAllDelegate();

	/** draw cross hair with linear color on each DrawHud call*/
	virtual void DrawHudCrossHair();

	/** draws a cross hair hit indicator when hits a target enemy */
	virtual void DrawHitFeedBackIndicator();

	/** draw pick up item info */
	virtual void DrawPickupItemInfo();

public:

	/** delegate call when weapon aims */
	UFUNCTION()
		virtual void OnAimWeapon();

	/** delegate call when weapon stop aim */
	UFUNCTION()
		virtual void OnStopAimWeapon();

	/** delegate call when weapon reload */
	UFUNCTION()
		virtual void OnReloadWeapon();

	/** delegate call when weapon finishes reload */
	UFUNCTION()
		virtual void OnFinishReloadWeapon();

	/** reset cross hair tint color to default */
	UFUNCTION()
		virtual void ResetCrossHairTintColor();

	/** set the cross hair tint with give color type */
	virtual void SetCrossHairTintColor(FLinearColor NewColor);

	/** set Last actor we see */
	virtual void SetLastSeenActor(AActor* NewActor);

	/** return last actor we see */
	inline virtual AActor* GetLastSeenActor()const {return LastSeenActor.Get();}

	/** return current ref weapon */
	inline AINSWeaponBase* GetCurrentWeapon()const;

	/** set current ref weapon */
	virtual void SetCurrentWeapon(AINSWeaponBase* CurrentNewWeapon);

	/** cast and return insPlayercontroller type  */
	inline AINSPlayerController* GetINSOwingPlayerController();

	virtual void DrawWaitingForRespawnMessage();

	virtual void DrawMatchPrepareMessage();

	virtual void SetPickupItemInfo(UTexture2D* NewItemTexture,bool ShowItemsStatus);

	virtual void DrawAmmoInfo();

	virtual void DrawHealth();

};
