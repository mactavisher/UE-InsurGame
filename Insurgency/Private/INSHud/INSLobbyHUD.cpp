// Fill out your copyright notice in the Description page of Project Settings.


#include "INSHud/INSLobbyHUD.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "UMG/Public/Blueprint/WidgetTree.h"

AINSLobbyHUD::AINSLobbyHUD(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	
}

void AINSLobbyHUD::BeginPlay()
{
	Super::BeginPlay();
	OwningLobbyController = Cast<AINSLobbyPlayerController>(GetOwningPlayerController());
	CreateLobbyMainWidget();
	LoadLobbyMainMenus();
}

void AINSLobbyHUD::CreateLobbyMainWidget()
{
	if(LobbyMainWidgetClass)
	{
		LobbyMainWidgetInstance = CreateWidget(OwningLobbyController,LobbyMainWidgetClass,"LobbyMain");
		LobbyMainWidgetInstance->AddToViewport(0);
	}
}

void AINSLobbyHUD::LoadLobbyMainMenus()
{
	TArray<FName> SlotNames;
	LobbyMainWidgetInstance->GetSlotNames(SlotNames);
	TArray< UWidget* > AllWidgets;
	LobbyMainWidgetInstance->WidgetTree->GetAllWidgets(AllWidgets);
//	int index;
	// UPanelWidget* Panel = LobbyMainWidgetInstance->WidgetTree->FindWidgetParent(LobbyMainWidgetInstance, index);
	// //GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Green, Panel->GetName());
	// //const TArray<UPanelSlot*> PanelSlots = Panel->GetSlots();
	// // UPanelSlot LobbyMainWidgetInstance->GetSlotNames()
	// // FString SlotDebugMessage;
	// // if(PanelSlots.Num()>0)
	// // {
	// // 	for (const UPanelSlot* const PanelSlot : PanelSlots) {
	// // 		SlotDebugMessage.Append(PanelSlot->GetName());
	// // 	}
	// // }
	// //LobbyMainWidgetInstance->SetContentForSlot("")
	// GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Green, SlotDebugMessage);
	// //GEngine->AddOnScreenDebugMessage(-5, 1.0, FColor::Green, Panel->GetName());
}
