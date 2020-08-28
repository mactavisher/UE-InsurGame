// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "INSHUDBase.generated.h"

class UINSWidget_CrossHair_Dot;
class UINSWidgetBase;
class AINSPlayerController;
class AINSWeaponBase;

USTRUCT(BlueprintType)
struct FDrawPlayerKilledInfo
{
	GENERATED_USTRUCT_BODY()
		friend class AINSHUDBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float LastDrawTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float DrawTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float DrawTimeEclapsed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		uint8 bAvailableforDrawing : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DrawMessage")
		FString DrawMessage;

public:
	FDrawPlayerKilledInfo()
		: LastDrawTime(0.f)
		, DrawTime(3.f)
		, DrawTimeEclapsed(0.f)
		, bAvailableforDrawing(true)
		, DrawMessage(TEXT("Kill"))
	{
	}

	void ResetDrawStatus()
	{
		LastDrawTime = 0.f;
		DrawTimeEclapsed = 0.f;
	}

	void SetIsAvailableForDrawing(bool bIsAvalilable)
	{
		bAvailableforDrawing = bIsAvalilable;
	}
};

USTRUCT(BlueprintType)
struct FDrawHitFeedBackIndicatorInfo
{
	GENERATED_USTRUCT_BODY()
		friend class AINSHUDBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D CenterScreen;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D LeftUpBegin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D LeftUpEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D RightUpBegin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D RightUpEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D LeftDownBegin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D LeftDownEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D RightDownBegin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D RightDownEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float DrawTimeEclapsed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DrawMessage")
		FLinearColor DrawColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DrawMessage")
		float LineLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DrawMessage")
		float BaseLineOffSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		uint8 bShowHitFeedBackIndicator : 1;

public:
	FDrawHitFeedBackIndicatorInfo()
		: DrawTimeEclapsed(0.f)
		, DrawColor(FLinearColor::White)
		, LineLength(5.f)
		, BaseLineOffSet(30.f)
		, bShowHitFeedBackIndicator(false)
	{
	}

	void CalculateCoord(FVector2D PivotPoint)
	{
		CenterScreen = PivotPoint;
		if (!CenterScreen.IsZero())
		{
			LeftUpBegin = CenterScreen + FVector2D(-BaseLineOffSet - LineLength, BaseLineOffSet + LineLength);
			LeftUpEnd = CenterScreen + FVector2D(-BaseLineOffSet, BaseLineOffSet);
			RightUpBegin = CenterScreen + FVector2D(BaseLineOffSet + LineLength, BaseLineOffSet + LineLength);
			RightUpEnd = CenterScreen + FVector2D(BaseLineOffSet, BaseLineOffSet);
			LeftDownBegin = CenterScreen + FVector2D(-BaseLineOffSet - LineLength, -BaseLineOffSet - LineLength);
			LeftDownEnd = CenterScreen + FVector2D(-BaseLineOffSet, -BaseLineOffSet);
			RightDownBegin = CenterScreen + FVector2D(BaseLineOffSet + LineLength, -BaseLineOffSet - LineLength);
			RightDownEnd = CenterScreen + FVector2D(+BaseLineOffSet, -BaseLineOffSet);
		}
	}
	void ResetDrawStatus()
	{
		bShowHitFeedBackIndicator = false;
		DrawTimeEclapsed = 0.f;
		DrawColor = FLinearColor::White;
		LineLength = 5.f;
		BaseLineOffSet = 30.f;
	}
};

USTRUCT(BlueprintType)
struct FDrawPlayerScoreInfo
{
	GENERATED_USTRUCT_BODY()
		friend class AINSHUDBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float LastDrawTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float DrawTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float DrawTimeEclapsed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float DrawScoreInterpSpeed;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ScoreToDraw")
		int32 ScoreToDraw;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ScoreToDraw")
		int32 CurrentFrameScoreValue;

public:
	FDrawPlayerScoreInfo()
		: LastDrawTime(0.f)
		, DrawTime(3.f)
		, DrawTimeEclapsed(0.f)
		, DrawScoreInterpSpeed(5.f)
		, ScoreToDraw(0)
		, CurrentFrameScoreValue(0)
	{
	}

	void ResetDrawStatus()
	{
		LastDrawTime = 0.f;
		DrawTime = 3.f;
		DrawTimeEclapsed = 0.f;
		DrawScoreInterpSpeed = 5.f;
		ScoreToDraw = 0;
		CurrentFrameScoreValue = 0;
	}
};
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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CrossHair")
		FLinearColor CrossHairDefaultTintColor;

	/** extra effect tint color ,for example , when contact with enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CrossHair")
		FLinearColor CrossHairThreatenTintColor;

	/** extra effect tint color ,for example , when contact with enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		uint8 bShowItemInfo : 1;

	/** extra effect tint color ,for example , when contact with enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		UTexture2D* ItemTexture;

	/** extra effect tint color ,for example , when contact with enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		uint8 DrawScoreStatus : 1;

	/** current weapon the player equipped */
	TWeakObjectPtr<AINSWeaponBase> CurrentWeapon;

	/** last actor we see */
	TWeakObjectPtr<AActor> LastSeenActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawPlayerKillList")
		TArray<FDrawPlayerKilledInfo> DrawPlayerKillInfos;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawPlayerKillList")
		FDrawPlayerScoreInfo DrawScoreInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawPlayerKillList")
		FDrawHitFeedBackIndicatorInfo DrawHitFeedBackInfo;


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
	inline virtual AActor* GetLastSeenActor()const { return LastSeenActor.Get(); }

	/** return current ref weapon */
	inline AINSWeaponBase* GetCurrentWeapon()const;

	/** set current ref weapon */
	virtual void SetCurrentWeapon(AINSWeaponBase* CurrentNewWeapon);

	/** cast and return insPlayercontroller type  */
	inline AINSPlayerController* GetINSOwingPlayerController();

	virtual void DrawWaitingForRespawnMessage();

	virtual void DrawMatchPrepareMessage();

	virtual void SetPickupItemInfo(UTexture2D* NewItemTexture, bool ShowItemsStatus);

	virtual void DrawAmmoInfo();

	virtual void DrawHealth();

	virtual void SetStartDrawScore(bool NewDrawScoreState, int32 InScoreForDrawing);
	virtual void SetStartDrawHitFeedBack(FLinearColor NewDrawColor);

	virtual void DrawScore();

	virtual void DrawWeaponFireMode();

	virtual void DrawTestInfo();

	virtual void DrawPlayerKill(const class APlayerState* Killer, const class APlayerState* Vimtim);

};
