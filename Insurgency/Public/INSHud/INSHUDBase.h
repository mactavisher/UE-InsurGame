// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "INSHUDBase.generated.h"

class UINSWidget_CrossHair_Dot;
class UINSWidgetBase;
class AINSPlayerController;
class AINSWeaponBase;
class UINSCrossHair;
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

/**
 * helper struct to track screen drawing a hit feed back indicator
 */
USTRUCT(BlueprintType)
struct FDrawHitFeedBackIndicatorInfo
{
	GENERATED_USTRUCT_BODY()

		friend class AINSHUDBase;

	/** center potion of the canvas */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D CenterScreen;

	/** Left up start part coordinate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D LeftUpBegin;

	/** Left up end part coordinate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D LeftUpEnd;

	/** right up start part coordinate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D RightUpBegin;

	/** right end part coordinate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D RightUpEnd;

	/** Left down start part coordinate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D LeftDownBegin;

	/** Left down end part coordinate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D LeftDownEnd;

	/** right down start part coordinate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D RightDownBegin;

	/** right down end part coordinate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		FVector2D RightDownEnd;

	/** time elapsed since last time we active drawing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeControll")
		float DrawTimeEclapsed;

	/** Draw Color */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DrawMessage")
		FLinearColor DrawColor;

	/** line length */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawMessage")
		float LineLength;

	/** Base off-set value to center of the screen */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawMessage")
		float BaseLineOffSet;

	/** Base off-set value to center of the screen */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawMessage")
		float LineOffSetInterpSpeed;

	/** is drawing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controll")
		uint8 bShowHitFeedBackIndicator : 1;

	/** line calculate method,if false, will use rotate line instead of simple add  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controll")
		uint8 bCoordinateSimpleAdd : 1;

public:

	FDrawHitFeedBackIndicatorInfo()
		: DrawTimeEclapsed(0.f)
		, DrawColor(FLinearColor::White)
		, LineLength(6.5f)
		, BaseLineOffSet(15.f)
		, LineOffSetInterpSpeed(0.75f)
		, bShowHitFeedBackIndicator(false)
		, bCoordinateSimpleAdd(true)
	{
	}

	/**
	 * @desc  given a point coordinate and rotate the vector with angle(degrees)
	 * @param Pivot             pivot
	 * @Param InVectorToRotate  InVectorToRotate
	 * @Param Degrees           degrees to rotate
	 */
	void RotatePoint(const FVector2D& Pivot, FVector2D& InVectorToRotate, const float Degrees)
	{
		const float X1 = InVectorToRotate.X - Pivot.X;
		const float Y1 = InVectorToRotate.Y - Pivot.Y;
		const float X2 = (X1 * FMath::Cos(Degrees) - Y1 * FMath::Sin(Degrees));
		const float Y2 = (X1 * FMath::Sin(Degrees) + Y1 * FMath::Cos(Degrees));
		InVectorToRotate.Set(X2 + Pivot.X, Y2 + Pivot.Y);
	}

	/**
	 * @desc  given a point coordinate and calculate the coordinate of each part During each draw frame
	 * @Param PivotPoint  Pivot
	 * 
	 */
	void CalculateCoord(FVector2D PivotPoint)
	{
		CenterScreen = PivotPoint;
		if (CenterScreen.IsZero())
		{
			return;
		}
		LeftUpBegin = CenterScreen + FVector2D(-BaseLineOffSet - LineLength, BaseLineOffSet + LineLength);
		LeftUpEnd = CenterScreen + FVector2D(-BaseLineOffSet, BaseLineOffSet);
		RightUpBegin = CenterScreen + FVector2D(BaseLineOffSet + LineLength, BaseLineOffSet + LineLength);
		RightUpEnd = CenterScreen + FVector2D(BaseLineOffSet, BaseLineOffSet);
		LeftDownBegin = CenterScreen + FVector2D(-BaseLineOffSet - LineLength, -BaseLineOffSet - LineLength);
		LeftDownEnd = CenterScreen + FVector2D(-BaseLineOffSet, -BaseLineOffSet);
		RightDownBegin = CenterScreen + FVector2D(BaseLineOffSet + LineLength, -BaseLineOffSet - LineLength);
		RightDownEnd = CenterScreen + FVector2D(BaseLineOffSet, -BaseLineOffSet);
	}

	/**
	 * @desc  set the base distance to the center pivot
	 * @Param NewOffSetValue  Distance Value
	 */
	void SetBaseLineOffSet(float NewOffSetValue)
	{
		BaseLineOffSet = NewOffSetValue;
	}

	/**
	 * @desc  set the line length be drawled
	 * @Param NewLineLength  the line length will be drawled
	 */
	void SetLineLength(float NewLineLength)
	{
		LineLength = NewLineLength;
	}

	/** reset the drawing property be used */
	void ResetDrawStatus()
	{
		bShowHitFeedBackIndicator = false;
		DrawTimeEclapsed = 0.f;
		DrawColor = FLinearColor::White;
		LineLength = 6.5f;
		BaseLineOffSet = 15.f;
		LineOffSetInterpSpeed = 0.75f;
	}
};

/**
 * helper struct to track screen drawing score
 */
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


	/** stores all the Widget instances that Managed by this HUD for later access purposes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widgets")
		TArray<UINSWidgetBase*> WidgetInstances;

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

	/** tracking killing drawing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawPlayerKillList")
		TArray<FDrawPlayerKilledInfo> DrawPlayerKillInfos;

	/** tracking drawing a score indicator */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawPlayerKillList")
		FDrawPlayerScoreInfo DrawScoreInfo;

	/** tracking hit feed back info indicator drawing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DrawPlayerKillList")
		FDrawHitFeedBackIndicatorInfo DrawHitFeedBackInfo;

	/** cache the player contorller of ins type */
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="OwningPlayerController")
	    AINSPlayerController* OwningINSPlayerController;


protected:
	/** ~~--------------------------------------------------------------
	  delegates                       --------------------------------*/

	/** Aiming delegate */
	FScriptDelegate WeaponAimDelegate;

	/** Stop Aiming delegate */
	FScriptDelegate WeaponStopAimDelegate;

	/** Weapon reload delegate */
	FScriptDelegate WeaponReloadDelegate;

	/** Weapon Finish reload delegate */
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

	/** Draw waiting for re-spawn message */
	virtual void DrawWaitingForRespawnMessage();

	/** Draw match prepare message */
	virtual void DrawMatchPrepareMessage();

	/** draw a pickup able item info when you are able to interact with */
	virtual void SetPickupItemInfo(UTexture2D* NewItemTexture, bool ShowItemsStatus);

	/** draw current weapon ammo info */
	virtual void DrawAmmoInfo();

	/** draw player deaths */
	virtual void DrawHealth();

	/** set this HUD to start draw score */
	virtual void SetStartDrawScore(bool NewDrawScoreState, int32 InScoreForDrawing);

	/** set this HUD to start draw score */
	virtual void SetStartDrawHitFeedBack(FLinearColor NewDrawColor);

	virtual void DrawScore();

	virtual AINSPlayerController* GetINSOwingPlayerController()const { return OwningINSPlayerController; }

	virtual void DrawWeaponFireMode();

	virtual void DrawTestInfo();

	virtual void DrawImmuneInfo();

	virtual class UCanvas* GetCanvas()const;

	virtual void DrawPlayerKill(const class APlayerState* Killer, const class APlayerState* Vimtim);

};
