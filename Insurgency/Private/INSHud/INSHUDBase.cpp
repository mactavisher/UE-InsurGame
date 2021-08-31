// Fill out your copyright notice in the Description page of Project Settings.


#include "INSHud/INSHUDBase.h"
#include "Engine/Canvas.h"
#include "TimerManager.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSGameModes/INSGameStateBase.h"
#ifndef GEngine
#include "Engine/Engine.h"
#endif

AINSHUDBase::AINSHUDBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bUsingHudCrossHair = true;
	StandardSizeX = 1920.f;
	StandardSizeY = 1080.f;
	CenterRadius = 5.f;
	CrossHairLineLength = 12.f;
	CrossHairDefaultTintColor = FLinearColor::White;
	CrossHairThreatenTintColor = FLinearColor::Red;
	CrossHairCurrentTintColor = CrossHairDefaultTintColor;
	bShowCrossHair = false;
	bShowItemInfo = false;
	DrawPlayerKillInfos.SetNum(5);
	for (uint8 i = 0; i < 5; i++)
	{
		DrawPlayerKillInfos.Add(FDrawPlayerKilledInfo());
	}
}

void AINSHUDBase::BeginPlay()
{
	Super::BeginPlay();
	OwningINSPlayerController = Cast<AINSPlayerController>(GetOwningPlayerController());
	CreateWidgetInstances();
}

void AINSHUDBase::DrawHUD()
{
	Super::DrawHUD();

	//HUD cross hair
	if (bShowCrossHair && bUsingHudCrossHair && CurrentWeapon.Get())
	{
		DrawHudCrossHair();
	}
	//Respawn Message if waiting for respawn
	AINSPlayerStateBase* MyPlayerState = PlayerOwner->PlayerState == nullptr ? nullptr : Cast<AINSPlayerStateBase>(PlayerOwner->PlayerState);
	if (MyPlayerState && MyPlayerState->GetIsWaitingForRespawn())
	{
		DrawWaitingForRespawnMessage();
	}
	//Prepare match Message
	AINSGameStateBase* CurrentGameState = PlayerOwner->GetWorld()->GetGameState<AINSGameStateBase>();
	if (CurrentGameState)
	{
		if (CurrentGameState->GetIsPreparingMatch())
		{
			DrawMatchPrepareMessage();
		}
	}
	DrawMyTeamInfo();
	if (bShowItemInfo)
	{
		DrawPickupItemInfo();
	}
	DrawAmmoInfo();
	DrawHealth();
	DrawWeaponFireMode();
	DrawScore();
	DrawHitFeedBackIndicator();
	DrawTestInfo();
	DrawImmuneInfo();
}


void AINSHUDBase::DrawMyTeamInfo()
{
	if (!OwningINSPlayerController)
	{
		return;
	}
	AINSPlayerStateBase* const MyPlayerState = OwningINSPlayerController->GetINSPlayerState();
	if (!MyPlayerState)
	{
		return;
	}
	const AINSTeamInfo* MyTeamInfo = MyPlayerState->GetPlayerTeam();
	FString MyTeamName("Team:");
	if (MyTeamInfo)
	{
		const ETeamType MyTeamType = MyTeamInfo->GetTeamType();
		switch (MyTeamType)
		{
		case ETeamType::ALLIE:MyTeamName.Append(TeamName::Allie.ToString()); break;
		case ETeamType::REBEL:MyTeamName.Append(TeamName::Rebel.ToString()); break;
		default: MyTeamName.Append("None"); break;
		}
		DrawText(MyTeamName, FLinearColor::White, Canvas->SizeX * 0.1f, Canvas->SizeY * 0.95f, GEngine->GetSmallFont(), 1.2f, false);
	}
}

void AINSHUDBase::CreateWidgetInstances()
{
	/*if (DotCrossHairWidgetClass)
	{
		DotCrossHairWidgetPtr = CreateWidget<UINSWidget_CrossHair_Dot>(GetOwningPlayerController(), DotCrossHairWidgetClass, FName(TEXT("Dot Cross Hair")));
		DotCrossHairWidgetPtr->SetOwningPlayer(PlayerOwner);
		DotCrossHairWidgetPtr->SetOwningLocalPlayer(PlayerOwner->GetLocalPlayer());
		DotCrossHairWidgetPtr->AddToViewport(0);
		DotCrossHairWidgetPtr->SetVisibility(ESlateVisibility::Visible);
	}*/
}


void AINSHUDBase::RemoveAllDelegate()
{
	WeaponAimDelegate.Unbind();
	WeaponStopAimDelegate.Unbind();
	WeaponReloadDelegate.Unbind();
	WeaponFinishReloadDelegate.Unbind();
}

void AINSHUDBase::DrawHudCrossHair()
{
	CurrentWeapon->DrawCrossHair(Canvas, FLinearColor::Red);
}

void AINSHUDBase::DrawHitFeedBackIndicator()
{
	if (DrawHitFeedBackInfo.bShowHitFeedBackIndicator)
	{
		DrawHitFeedBackInfo.CalculateCoord(FVector2D(Canvas->ClipX / 2.f, Canvas->ClipY / 2.f));
		DrawLine(DrawHitFeedBackInfo.LeftUpBegin.X, DrawHitFeedBackInfo.LeftUpBegin.Y,
			DrawHitFeedBackInfo.LeftUpEnd.X, DrawHitFeedBackInfo.LeftUpEnd.Y,
			DrawHitFeedBackInfo.DrawColor,
			2.f);

		DrawLine(DrawHitFeedBackInfo.RightUpBegin.X, DrawHitFeedBackInfo.RightUpBegin.Y,
			DrawHitFeedBackInfo.RightUpEnd.X, DrawHitFeedBackInfo.RightUpEnd.Y,
			DrawHitFeedBackInfo.DrawColor,
			2.f);

		DrawLine(DrawHitFeedBackInfo.LeftDownBegin.X, DrawHitFeedBackInfo.LeftDownBegin.Y,
			DrawHitFeedBackInfo.LeftDownEnd.X, DrawHitFeedBackInfo.LeftDownEnd.Y,
			DrawHitFeedBackInfo.DrawColor,
			2.f);

		DrawLine(DrawHitFeedBackInfo.RightDownBegin.X, DrawHitFeedBackInfo.RightDownBegin.Y,
			DrawHitFeedBackInfo.RightDownEnd.X, DrawHitFeedBackInfo.RightDownEnd.Y,
			DrawHitFeedBackInfo.DrawColor,
			2.f);

		DrawHitFeedBackInfo.BaseLineOffSet = DrawHitFeedBackInfo.BaseLineOffSet * 0.9f;

		if (DrawHitFeedBackInfo.BaseLineOffSet <= 8.f)
		{
			DrawHitFeedBackInfo.BaseLineOffSet = 8.f;
			DrawHitFeedBackInfo.ResetDrawStatus();
		}
	}
}

void AINSHUDBase::DrawPickupItemInfo()
{
	if (ItemTexture)
	{
		DrawTexture(ItemTexture, Canvas->ClipX / 2, Canvas->ClipY * 0.8f
			, Canvas->ClipX, Canvas->ClipY, ItemTexture->GetSizeX()
			, ItemTexture->GetSizeY(), ItemTexture->GetSizeX()
			, ItemTexture->GetSizeY());
	}
	FString DrawMessage("See pickupable Weapon");
	DrawText(DrawMessage, FLinearColor::White, Canvas->ClipX / 0.4, Canvas->ClipY / 0.5f, GEngine->GetMediumFont(), 1.f, false);
}

void AINSHUDBase::OnAimWeapon()
{
	bShowCrossHair = false;
}

void AINSHUDBase::OnStopAimWeapon()
{
	bShowCrossHair = true;
}

void AINSHUDBase::OnReloadWeapon()
{
	bShowCrossHair = false;
}

void AINSHUDBase::OnFinishReloadWeapon()
{
	bShowCrossHair = true;
}

void AINSHUDBase::ResetCrossHairTintColor()
{
	CrossHairCurrentTintColor = CrossHairDefaultTintColor;
	GetWorldTimerManager().ClearTimer(Timer_ResetCrossHairTintColor);
}

void AINSHUDBase::SetCrossHairTintColor(FLinearColor NewColor)
{
	CrossHairCurrentTintColor = NewColor;
}

void AINSHUDBase::SetLastSeenActor(class AActor* NewActor)
{
	this->LastSeenActor = NewActor;
}

AINSWeaponBase* AINSHUDBase::GetCurrentWeapon() const
{
	return CurrentWeapon.Get();
}

void AINSHUDBase::SetCurrentWeapon(class AINSWeaponBase* CurrentNewWeapon)
{
	RemoveAllDelegate();
	CurrentWeapon = CurrentNewWeapon;
	bShowCrossHair = true;
}

class AINSPlayerController* AINSHUDBase::GetINSOwingPlayerController()
{
	return Cast<AINSPlayerController>(GetOwningPlayerController());
}

void AINSHUDBase::DrawWaitingForRespawnMessage()
{
	FString RespawnMessage;
	RespawnMessage.Append("Waiting For Respawn....");
	const AINSPlayerStateBase* const MyPlayerState = CastChecked<AINSPlayerStateBase>(GetINSOwingPlayerController()->PlayerState);
	RespawnMessage.Append(FString::FromInt(MyPlayerState->GetReplicatedRespawnRemainingTime()));
	RespawnMessage.Append(" Seconds");
	DrawText(RespawnMessage, FLinearColor::White, Canvas->SizeX * 0.45f, Canvas->SizeY * 0.8f, GEngine->GetSmallFont(), 1.f, false);
}

void AINSHUDBase::DrawMatchPrepareMessage()
{
	FString PrepareMessage;
	PrepareMessage.Append("match starts in ....");
	const AINSGameStateBase* const CurrentGameState = GetWorld()->GetGameState<AINSGameStateBase>();
	PrepareMessage.Append(FString::FromInt(CurrentGameState->GetReplicatedMatchPrepareRemainingTime()));
	PrepareMessage.Append(" Seconds");
	DrawText(PrepareMessage, FLinearColor::White, Canvas->SizeX * 0.4f, Canvas->SizeY * 0.2f, GEngine->GetMediumFont(), 1.f, false);
}

void AINSHUDBase::SetPickupItemInfo(UTexture2D* NewItemTexture, bool ShowItemsStatus)
{
	bShowItemInfo = ShowItemsStatus;
	ItemTexture = ItemTexture;
}

void AINSHUDBase::DrawAmmoInfo()
{
	if (CurrentWeapon.Get())
	{
		FString AmmoMessage;
		AmmoMessage.Append(FString::FromInt(CurrentWeapon.Get()->CurrentClipAmmo));
		AmmoMessage.Append("/");
		AmmoMessage.Append(FString::FromInt(CurrentWeapon.Get()->AmmoLeft));
		DrawText(AmmoMessage, FLinearColor::White, Canvas->SizeX * 0.9f, Canvas->SizeY * 0.95f, GEngine->GetMediumFont(), 1.f, false);
	}
}

void AINSHUDBase::DrawHealth()
{
	if (GetINSOwingPlayerController()->GetINSPlayerCharacter() && !GetINSOwingPlayerController()->GetINSPlayerCharacter()->GetIsDead())
	{
		FString AmmoMessage;
		AmmoMessage.Append("HP:");
		AmmoMessage.Append(FString::SanitizeFloat(GetINSOwingPlayerController()->GetINSPlayerCharacter()->GetCharacterCurrentHealth()));
		const AINSPlayerCharacter* PlayerCharacter = GetINSOwingPlayerController()->GetINSPlayerCharacter();
		FLinearColor HealthColor = PlayerCharacter->GetIsLowHealth() ? FLinearColor::Red : FLinearColor::White;
		DrawText(AmmoMessage, HealthColor, Canvas->SizeX * 0.9f, Canvas->SizeY * 0.91f, GEngine->GetMediumFont(), 1.f, false);
	}
}


void AINSHUDBase::SetStartDrawScore(bool NewDrawState, int32 InScoreForDrawing)
{
	//DrawScoreInfo.ResetDrawStatus();
	DrawScoreStatus = NewDrawState;
	DrawScoreInfo.ScoreToDraw = InScoreForDrawing;

}

void AINSHUDBase::SetStartDrawHitFeedBack(FLinearColor NewDrawColor)
{
	DrawHitFeedBackInfo.ResetDrawStatus();
	DrawHitFeedBackInfo.DrawColor = NewDrawColor;
	DrawHitFeedBackInfo.bShowHitFeedBackIndicator = true;
}

void AINSHUDBase::DrawScore()
{
	if (DrawScoreStatus)
	{
		const float CurrentWorldTime = GetWorld()->GetTimeSeconds();
		DrawScoreInfo.CurrentFrameScoreValue += FMath::CeilToInt(GetWorld()->GetDeltaSeconds() * DrawScoreInfo.DrawScoreInterpSpeed);
		if (DrawScoreInfo.CurrentFrameScoreValue >= DrawScoreInfo.ScoreToDraw)
		{
			DrawScoreInfo.CurrentFrameScoreValue = DrawScoreInfo.ScoreToDraw;
		}
		DrawText(FString(TEXT("+")).Append(FString::FromInt(DrawScoreInfo.CurrentFrameScoreValue)), FLinearColor::Yellow, Canvas->SizeX * 0.55f, Canvas->SizeY * 0.50f, GEngine->GetSmallFont(), 1.f, false);
		DrawScoreInfo.DrawTimeEclapsed += GetWorld()->GetDeltaSeconds();
		DrawScoreInfo.DrawScoreInterpSpeed = DrawScoreInfo.DrawScoreInterpSpeed * 0.8f;
		if (DrawScoreInfo.DrawScoreInterpSpeed <= 1.f)
		{
			DrawScoreInfo.DrawScoreInterpSpeed = 1.f;
		}
		if (DrawScoreInfo.DrawTime <= DrawScoreInfo.DrawTimeEclapsed)
		{
			DrawScoreStatus = false;
			DrawScoreInfo.ResetDrawStatus();
		}
	}

}

void AINSHUDBase::DrawWeaponFireMode()
{
	if (CurrentWeapon.Get() && GetINSOwingPlayerController()->GetINSPlayerCharacter()&&!GetINSOwingPlayerController()->GetINSPlayerCharacter()->GetIsDead())
	{
		FString FireMode;
		EWeaponFireMode CurrentWeaponFireMode = CurrentWeapon->GetCurrentWeaponFireMode();
		switch (CurrentWeaponFireMode)
		{
		case EWeaponFireMode::SEMI:FireMode.Append("Semi"); break;
		case EWeaponFireMode::SEMIAUTO:FireMode.Append("Semi-Auto"); break;
		case EWeaponFireMode::FULLAUTO:FireMode.Append("Full-Auto"); break;
		default:
			break;
		}
		DrawText(FireMode, FLinearColor::White, Canvas->SizeX * 0.95f, Canvas->SizeY * 0.95f, GEngine->GetSmallFont(), 1.f, false);
	}
}

void AINSHUDBase::DrawTestInfo()
{
	DrawText(TEXT("Development Prototype Test"), FLinearColor::White, 10.f, 10.f, GEngine->GetSmallFont(), 1.f, false);
}

void AINSHUDBase::DrawImmuneInfo()
{
	const AINSPlayerCharacter* PlayerCharacter = GetINSOwingPlayerController()->GetINSPlayerCharacter();
	if (PlayerCharacter && PlayerCharacter->GetIsDamageImmune())
	{
		FString DamageImmuneMessage;
		DamageImmuneMessage.Append("Damage Immune time Left:");
		DamageImmuneMessage.Append(FString::FromInt(PlayerCharacter->GetDamageImmuneTimeLeft()));
		FLinearColor HealthColor = PlayerCharacter->GetIsLowHealth() ? FLinearColor::Red : FLinearColor::White;
		DrawText(DamageImmuneMessage, FLinearColor::White, Canvas->SizeX * 0.45f, Canvas->SizeY * 0.9f, GEngine->GetMediumFont(), 1.f, false);
	}
}

class UCanvas* AINSHUDBase::GetCanvas() const
{
	return Canvas;
}

void AINSHUDBase::DrawPlayerKill(const class APlayerState* Killer, const class APlayerState* Vimtim)
{
	const uint8 Num = DrawPlayerKillInfos.Num();

}
