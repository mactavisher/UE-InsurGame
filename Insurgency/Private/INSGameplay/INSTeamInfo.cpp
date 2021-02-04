// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameplay/INSTeamInfo.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#ifndef AINSPlayerStateBase
#include "INSCharacter/INSPlayerStateBase.h"
#endif
#include "INSGameModes/INSGameModeBase.h"
#include "INSCharacter/INSPlayerController.h"
AINSTeamInfo::AINSTeamInfo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	SetReplicates(true);
	bAlwaysRelevant = true;
	SetReplicatingMovement(false);
	NetUpdateFrequency = 1;
	// Note: this is very important to set to false. Though all replication infos are spawned at run time, during seamless travel
	// they are held on to and brought over into the new world. In ULevel::InitializeActors, these PlayerStates may be treated as map/startup actors
	// and given static NetGUIDs. This also causes their deletions to be recorded and sent to new clients, which if unlucky due to name conflicts,
	// may end up deleting the new PlayerStates they had just spaned.
	bNetLoadOnClient = false;
}

void AINSTeamInfo::BeginPlay()
{
	Super::BeginPlay();
}

void AINSTeamInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSTeamInfo, ThisTeamType);
	DOREPLIFETIME(AINSTeamInfo, TeamScore);
}

void AINSTeamInfo::SetTeamType(ETeamType NewTeamType)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ThisTeamType = NewTeamType;
		OnRep_TeamType();
	}
}

void AINSTeamInfo::OnRep_TeamType()
{

}

void AINSTeamInfo::AddPlayerToThisTeam(class AINSPlayerStateBase* NewPlayer)
{
	if (!isTeamFull() && NewPlayer != nullptr)
	{
		TeamMembers.AddUnique(NewPlayer);
	}
}

void AINSTeamInfo::SortPlayersByScore()
{
// 	StrArr.Sort([](const FString& A, const FString& B) {
// 		return A.Len() > B.Len();
// 	});
}

void AINSTeamInfo::RemovePlayer(class AINSPlayerStateBase* PlayerToRemove)
{
	TeamMembers.Remove(PlayerToRemove);
}

bool AINSTeamInfo::isTeamFull()
{
	const AINSGameModeBase* const CurrentGameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
	if (CurrentGameMode == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Team info related game mode dosen't exist"));
		return true;
	}
	return GetCurrentTeamPlayers() < CurrentGameMode->GetMaxSingleTeamPlayers();
}

