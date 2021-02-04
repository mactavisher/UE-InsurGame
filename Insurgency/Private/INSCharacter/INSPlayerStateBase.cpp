// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSPlayerStateBase.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "Insurgency/Insurgency.h"
#include "INSGameplay/INSTeamInfo.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSGameModes/INSGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#ifndef AINSHUDBase
#include "INSHud/INSHUDBase.h"
#endif
#ifndef AINSTeamInfo
#include "INSGameplay/INSTeamInfo.h"
#endif
AINSPlayerStateBase::AINSPlayerStateBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bIsWaitingForRespawn = false;
	RespawnRemainingTime = 0.f;
	CachedDamageInfoMaxSize = 5;
	CachedDamageInfos.SetNum(CachedDamageInfoMaxSize);
	Kills = 0;
	Death = 0;
}
void AINSPlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSPlayerStateBase, PlayerTeam);
	DOREPLIFETIME(AINSPlayerStateBase, MyScore);
	DOREPLIFETIME(AINSPlayerStateBase, Kills);
	DOREPLIFETIME(AINSPlayerStateBase, Death);
	DOREPLIFETIME(AINSPlayerStateBase, bIsWaitingForRespawn);
	DOREPLIFETIME(AINSPlayerStateBase, ReplicatedRespawnRemainingTime);
}

void AINSPlayerStateBase::OnRep_Score()
{
	if (GetPawn()&& GetPawn()->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::SanitizeFloat(GetScore()));
	}
}

void AINSPlayerStateBase::BeginPlay()
{
	Super::BeginPlay();
	AINSGameStateBase*CurrentGameState = GetWorld()->GetGameState<AINSGameStateBase>();
	if (CurrentGameState)
	{
		CurrentGameState->OnKill.AddDynamic(this, &AINSPlayerStateBase::OnPlayerKill);
		CurrentGameState->OnDamage.AddDynamic(this, &AINSPlayerStateBase::OnPlayerDamage);
	}
}

void AINSPlayerStateBase::OnRep_TeamInfo()
{
	
}

void AINSPlayerStateBase::OnRep_MyScore()
{
	
}

void AINSPlayerStateBase::OnRep_RespawnRemainingTime()
{

}


void AINSPlayerStateBase::SetPlayerTeam(class AINSTeamInfo* NewTeam)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayerTeam = NewTeam;
		OnRep_TeamInfo();
	}
}

void AINSPlayerStateBase::UpdateRepliatedRespawnRemaingTime()
{

}

void AINSPlayerStateBase::ReceiveHitInfo(const struct FTakeHitInfo TakeHitInfo)
{
	
}

void AINSPlayerStateBase::PlayerScore(int32 ScoreToAdd)
{

}

void AINSPlayerStateBase::OnPlayerKill(class APlayerState* Killer, class APlayerState* Victim, int32 KillerScore, bool bIsTeamDamage)
{

}

void AINSPlayerStateBase::OnPlayerDamage(class APlayerState* Killer, class APlayerState* Victim, int32 DamagaCauserScore, bool bIsTeamDamage)
{

}

void AINSPlayerStateBase::TickRespawnTime()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (bIsWaitingForRespawn)
		{
			RespawnRemainingTime -= 1.f;
			ReplicatedRespawnRemainingTime = (uint8)FMath::CeilToInt(RespawnRemainingTime);
			if (RespawnRemainingTime <= 0.f)
			{
				AINSPlayerController* OwnerPlayer = CastChecked<AINSPlayerController>(GetOwner());
				if (OwnerPlayer)
				{
					OwnerPlayer->ReceiveStartRespawn();
					GetWorldTimerManager().ClearTimer(RespawnTimer);
					bIsWaitingForRespawn = false;
					ReplicatedRespawnRemainingTime = 0;
					RespawnRemainingTime = 0.f;
				}
			}
		}
	}
}

void AINSPlayerStateBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AINSPlayerStateBase::ReceivePlayerDeath(AINSPlayerController* DeadPlayer)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DeadPlayer->UnPossess();
		bIsWaitingForRespawn = true;
		const AINSGameStateBase* CurrentGameState = Cast<AINSGameStateBase>(GetWorld()->GetGameState());
		RespawnRemainingTime = CurrentGameState->GetRespawnTime();
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &AINSPlayerStateBase::TickRespawnTime, 1.f, true, 1.f);
	}
}
