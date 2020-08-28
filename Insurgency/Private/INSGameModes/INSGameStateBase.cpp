// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameModes/INSGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "INSCharacter/INSPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogINSGameState);

AINSGameStateBase::AINSGameStateBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bShouldDropWeaponWhenPlayerDead = true;
	bAllowFire = false;
	bAllowMove = false;
	bIsMatchPrepare = false;
	bMatchPrepareFinished = false;
}
void AINSGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSGameStateBase, bAllowFire);
	DOREPLIFETIME(AINSGameStateBase, bAllowMove);
	DOREPLIFETIME(AINSGameStateBase, ReSpawnTime);
	DOREPLIFETIME(AINSGameStateBase, TerroristTeam);
	DOREPLIFETIME(AINSGameStateBase, CTTeam);
	DOREPLIFETIME(AINSGameStateBase, bIsMatchPrepare);
	DOREPLIFETIME(AINSGameStateBase, bMatchPrepareFinished);
	DOREPLIFETIME(AINSGameStateBase, ReplicatedMatchPrepareRemainingTime);
	DOREPLIFETIME_CONDITION(AINSGameStateBase, bShouldDropWeaponWhenPlayerDead, COND_InitialOnly);
}

void AINSGameStateBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AINSGameStateBase::OnRep_PrepareMath()
{

}

void AINSGameStateBase::ClientsReceiveKillEvent_Implementation(class APlayerState* Killer, class APlayerState* Victim, int32 KillerScore, bool bIsTeamDamage)
{
	//OnKill.Broadcast(Killer->PlayerState, Victim->PlayerState, KillerScore, bIsTeamDamage);
	if (Killer->GetClass()->IsChildOf(APlayerState::StaticClass()))
	{
		//get the local machine's player
		AINSPlayerController* LocalPC = Cast<AINSPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (LocalPC)
		{
			LocalPC->ReceiveGameKills(Killer, Victim, KillerScore, bIsTeamDamage);
		}
	}
}

bool AINSGameStateBase::ClientsReceiveKillEvent_Validate(class APlayerState* KillerState, class APlayerState* VictimState, int32 KillerScore, bool bIsTeamDamage)
{
	return true;
}

void AINSGameStateBase::OnPlayerKilled(class AController* Killer, class AController* Victim, int32 KillerScore, bool bIsTeamDamage)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (bIsTeamDamage)
		{
			KillerScore += 100;
		}
		AINSPlayerStateBase* KillerPlayerState = Killer->GetPlayerState<AINSPlayerStateBase>();
		if (KillerPlayerState)
		{
			KillerPlayerState->PlayerScore(KillerScore);
			KillerPlayerState->GetPlayerTeam()->AddTeamScore(KillerScore);
		}
		ClientsReceiveKillEvent(Killer->PlayerState, Victim->PlayerState, KillerScore, bIsTeamDamage);
		if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
		{
			ClientsReceiveKillEvent_Implementation(Killer->PlayerState, Killer->PlayerState, KillerScore, bIsTeamDamage);
		}
	}
}


void AINSGameStateBase::OnPlayerDamaged(class AController* DamageInstigtor, class AController* Victim, float DamageAmount, bool bIsTeamDamage)
{
	if (DamageInstigtor->GetClass()->IsChildOf(AINSPlayerController::StaticClass()))
	{
		AINSPlayerController* const PC = Cast<AINSPlayerController>(DamageInstigtor);
		if (PC)
		{
			PC->ClientReceiveCauseDamage(Victim, DamageAmount, bIsTeamDamage);
		}
	}
}

void AINSGameStateBase::OnRep_FinishePreparing()
{

}

void AINSGameStateBase::OnRep_PreparingRemainingTime()
{
	if (ReplicatedMatchPrepareRemainingTime >= 0 && ReplicatedMatchPrepareRemainingTime <= 3)
	{
		UGameplayStatics::SpawnSound2D(GetWorld(), ClockTickingSound);
	}
	if (ReplicatedMatchPrepareRemainingTime == 0)
	{
		UGameplayStatics::SpawnSound2D(GetWorld(), GameModeSound);
	}
	if (ReplicatedMatchPrepareRemainingTime == 0 && GameBGMSound)
	{
		UGameplayStatics::UGameplayStatics::SpawnSound2D(GetWorld(), GameBGMSound);
	}
}

void AINSGameStateBase::OnRep_GameType()
{

}
