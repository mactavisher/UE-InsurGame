// Fill out your copyright notice in the Description page of Project Settings.


#include "INSHud/INSHUDBase.h"
#include "Engine/Canvas.h"
#include "TimerManager.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSWidgets/INSWidget_CrossHair_Dot.h"
#include "INSGameModes/INSGameStateBase.h"

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
}

void AINSHUDBase::BeginPlay()
{
	Super::BeginPlay();
	CreateWidgetInstances();
}

void AINSHUDBase::DrawHUD()
{
	Super::DrawHUD();
	//HUD cross hair
	if (bUsingHudCrossHair&&CurrentWeapon.Get() && bShowCrossHair&&CurrentWeapon.Get()->GetWeaponCurrentState()!=EWeaponState::RELOADIND)
	{
		DrawHudCrossHair();
	}
	//Respawn Message if waiting for respawn
    AINSPlayerStateBase* MyPlayerState = PlayerOwner->PlayerState==nullptr?nullptr: Cast<AINSPlayerStateBase>(PlayerOwner->PlayerState);
	if (MyPlayerState&&MyPlayerState->GetIsWaitingForRespawn())
	{
		DrawWaitingForRespawnMessage();
	}
	//Prepare match Message
	AINSGameStateBase*CurrentGameState = PlayerOwner->GetWorld()->GetGameState<AINSGameStateBase>();
	if (CurrentGameState)
	{
		if (CurrentGameState->GetIsPreparingMatch())
		{
			DrawMatchPrepareMessage();
		}
	}
	DrawMyTeamInfo();
}

void AINSHUDBase::DrawMyTeamInfo()
{
	AINSPlayerStateBase* MyPlayerState = PlayerOwner->PlayerState == nullptr ? nullptr : Cast<AINSPlayerStateBase>(PlayerOwner->PlayerState);
	if (MyPlayerState)
	{
		FString MyTeamName("Team:");
		const AINSTeamInfo* MyTeamInfo = MyPlayerState->GetPlayerTeam();
		if (MyTeamInfo)
		{
			const ETeamType MyTeamType = MyTeamInfo->GetTeamType();
			switch (MyTeamType)
			{
			case ETeamType::CT:MyTeamName.Append("CT"); break;
			case ETeamType::T:MyTeamName.Append("T"); break;
			default: MyTeamName.Append("None"); break;
			}
			DrawText(MyTeamName, FLinearColor::White, Canvas->SizeX*0.1f, Canvas->SizeY*0.8f, GEngine->GetLargeFont(), 1.2f, false);
		}
	}
}

void AINSHUDBase::CreateWidgetInstances()
{
	if (DotCrossHairWidgetClass)
	{
		DotCrossHairWidgetPtr = CreateWidget<UINSWidget_CrossHair_Dot>(GetOwningPlayerController(), DotCrossHairWidgetClass, FName(TEXT("Dot Cross Hair")));
		DotCrossHairWidgetPtr->SetOwningPlayer(PlayerOwner);
		DotCrossHairWidgetPtr->SetOwningLocalPlayer(PlayerOwner->GetLocalPlayer());
		DotCrossHairWidgetPtr->AddToViewport(0);
		DotCrossHairWidgetPtr->SetVisibility(ESlateVisibility::Visible);
	}
}

void AINSHUDBase::BindDelegate()
{
	if (CurrentWeapon.Get())
	{
		WeaponAimDelegate.BindUFunction(this, TEXT("OnAimWeapon"));
		WeaponStopAimDelegate.BindUFunction(this, TEXT("OnStopAimWeapon"));
		WeaponReloadDelegate.BindUFunction(this, TEXT("OnReloadWeapon"));
		WeaponFinishReloadDelegate.BindUFunction(this, TEXT("OnFinishReloadWeapon"));
		CurrentWeapon->OnWeaponAim.AddUnique(WeaponAimDelegate);
		CurrentWeapon->OnStopWeaponAim.AddUnique(WeaponStopAimDelegate);
		CurrentWeapon->OnWeaponStartReload.AddUnique(WeaponReloadDelegate);
		CurrentWeapon->OnFinishReloadWeapon.AddUnique(WeaponFinishReloadDelegate);
	}
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
	const float ScaleX = Canvas->SizeX / StandardSizeX;
	const float ScaleY = Canvas->SizeY / StandardSizeY;
	const float WeaponSpreadModifier = CurrentWeapon.Get()->GetWeaponCurrentSpread();
	float BiasX = (CenterRadius + WeaponSpreadModifier)*ScaleX*2.f;
	float BiasY = (CenterRadius + WeaponSpreadModifier)* ScaleY*2.f;
	if (BiasX >= 400.f*ScaleX)
	{
		BiasX = 400.f*ScaleX;
	}
	if (BiasY >= 400.f*ScaleY)
	{
		BiasY = 400.f*ScaleY;
	}
	const float LeftLineCoordX = Canvas->SizeX / 2 - BiasX;
	const float LeftLineEndCoordX = LeftLineCoordX - CrossHairLineLength * ScaleX;
	const float LeftLineCoordY = Canvas->SizeY / 2;
	const float RightLineCoordX = Canvas->SizeX / 2 + BiasX;
	const float RightLineEndCoordX = RightLineCoordX + CrossHairLineLength * ScaleX;
	const float RightLineCoordY = Canvas->SizeY / 2;
	const float UpLineCoordX = Canvas->SizeX / 2;
	const float UplineCoordY = Canvas->SizeY / 2 - BiasY;
	const float UplineEndCoordY = UplineCoordY - CrossHairLineLength * ScaleY;
	const float DownLineCoordX = Canvas->SizeX / 2;
	const float DownlienCoordY = Canvas->SizeY / 2 + BiasY;
	const float DownlineEndCoordY = DownlienCoordY + CrossHairLineLength * ScaleY;
	if (GetINSOwingPlayerController() && GetINSOwingPlayerController()->HasSeeEnemy())
	{
		//tint cross red if see enemy
		CrossHairCurrentTintColor = CrossHairThreatenTintColor;
	}
	//draw left CrossHair part
	DrawLine(LeftLineCoordX, LeftLineCoordY, LeftLineEndCoordX, LeftLineCoordY, CrossHairCurrentTintColor, 1.2f);
	//Draw right CrossHair part
	DrawLine(RightLineCoordX, LeftLineCoordY, RightLineEndCoordX, LeftLineCoordY, CrossHairCurrentTintColor, 1.2f);
	//draw left CrossHair part
	DrawLine(UpLineCoordX, UplineCoordY, UpLineCoordX, UplineEndCoordY, CrossHairCurrentTintColor, 1.2f);
	//Draw right CrossHair part
	DrawLine(DownLineCoordX, DownlienCoordY, DownLineCoordX, DownlineEndCoordY, CrossHairCurrentTintColor, 1.2f);
}

void AINSHUDBase::DrawHitFeedBackIndicator()
{
	FVector2D LeftUpIndicatorStartCoordinate(Canvas->SizeX / 2.f, Canvas->SizeY / 2.f);
	FVector2D LeftUpIndicatorEndCoordinaet;
	FVector2D LeftDownIndicatorStartCoordinate;
	FVector2D leftDownIndicatorEndCoordinate;
	FVector2D RightUpIndicatorStartCoordinate;
	FVector2D RightUpIndicatorEndCoordinate;
	FVector2D RightDownIndicatorStartCoordinate;
	FVector2D RightDownIndicatorEndCoordinate;
}

void AINSHUDBase::OnAimWeapon()
{
	if (DotCrossHairWidgetPtr)
	{
		DotCrossHairWidgetPtr->SetVisibility(ESlateVisibility::Hidden);
	}
	bShowCrossHair = false;
}

void AINSHUDBase::OnStopAimWeapon()
{
	if (DotCrossHairWidgetPtr)
	{
		DotCrossHairWidgetPtr->SetVisibility(ESlateVisibility::Visible);
	}
	bShowCrossHair = true;
}

void AINSHUDBase::OnReloadWeapon()
{
	if (DotCrossHairWidgetPtr)
	{
		DotCrossHairWidgetPtr->SetVisibility(ESlateVisibility::Hidden);
	}
	bShowCrossHair = false;
}

void AINSHUDBase::OnFinishReloadWeapon()
{
	if (DotCrossHairWidgetPtr)
	{
		DotCrossHairWidgetPtr->SetVisibility(ESlateVisibility::Visible);
	}
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
	if (CurrentWeapon.Get())
	{
		BindDelegate();
	}
	if (!CurrentWeapon.Get())
	{
		RemoveAllDelegate();
	}
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
	UFont* Font = GEngine->GetLargeFont();
	RespawnMessage.Append(FString::FromInt(MyPlayerState->GetReplicatedRespawnRemainingTime()));
	RespawnMessage.Append(" Seconds");
	DrawText(RespawnMessage, FLinearColor::White, Canvas->SizeX*0.45f, Canvas->SizeY*0.8f, Font,1.f,false);
}

void AINSHUDBase::DrawMatchPrepareMessage()
{
	FString PrepareMessage;
	PrepareMessage.Append("match starts in ....");
	const AINSGameStateBase* const CurrentGameState =GetWorld()->GetGameState<AINSGameStateBase>();
	PrepareMessage.Append(FString::FromInt(CurrentGameState->GetReplicatedMatchPrepareRemainingTime()));
	PrepareMessage.Append(" Seconds");
	DrawText(PrepareMessage, FLinearColor::White, Canvas->SizeX*0.4f, Canvas->SizeY*0.2f,GEngine->GetLargeFont(),2.f,false);
}
