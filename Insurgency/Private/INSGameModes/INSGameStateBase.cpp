// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameModes/INSGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"


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

void AINSGameStateBase::OnRep_FinishePreparing()
{

}

void AINSGameStateBase::OnRep_PreparingRemainingTime()
{
	if (ReplicatedMatchPrepareRemainingTime >=0&&ReplicatedMatchPrepareRemainingTime<=3)
	{
		UGameplayStatics::SpawnSound2D(GetWorld(), ClockTickingSound);
	}
	if (ReplicatedMatchPrepareRemainingTime ==0)
	{
		UGameplayStatics::SpawnSound2D(GetWorld(), GameModeSound);
	}
	if (ReplicatedMatchPrepareRemainingTime == 0&&GameBGMSound)
	{
		UGameplayStatics::UGameplayStatics::SpawnSound2D(GetWorld(), GameBGMSound);
	}
}

void AINSGameStateBase::OnRep_GameType()
{

}
