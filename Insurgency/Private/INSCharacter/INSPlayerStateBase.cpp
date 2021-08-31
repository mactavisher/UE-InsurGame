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

DEFINE_LOG_CATEGORY(LogINSPlayerState);

AINSPlayerStateBase::AINSPlayerStateBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsWaitingForRespawn = false;
	RespawnRemainingTime = 0.f;
	CachedDamageInfoMaxSize = 5;
	CachedDamageInfos.SetNum(CachedDamageInfoMaxSize);
	Kills = 0;
	Deaths = 0;
	ReplicatedRespawnRemainingTime = 0.f;
	MissTakeKill = 0;
	KDRatio = 0.f;
	PlayerTeam = nullptr;
}

void AINSPlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSPlayerStateBase, PlayerTeam);
	DOREPLIFETIME(AINSPlayerStateBase, Kills);
	DOREPLIFETIME(AINSPlayerStateBase, Deaths);
	DOREPLIFETIME(AINSPlayerStateBase, bIsWaitingForRespawn);
	DOREPLIFETIME(AINSPlayerStateBase, ReplicatedRespawnRemainingTime);
}

void AINSPlayerStateBase::OnRep_Score()
{
	if (GetPawn() && GetPawn()->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::SanitizeFloat(GetScore()));
	}
}

void AINSPlayerStateBase::BeginPlay()
{
	Super::BeginPlay();
	AINSGameStateBase* CurrentGameState = GetWorld()->GetGameState<AINSGameStateBase>();
}

void AINSPlayerStateBase::OnRep_TeamInfo()
{
}

void AINSPlayerStateBase::OnRep_Deaths()
{
	UpdateKDRatio();
}

void AINSPlayerStateBase::OnRep_Kills()
{
	UpdateKDRatio();
}

void AINSPlayerStateBase::OnRep_MistakeKill()
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

void AINSPlayerStateBase::UpdateReplicatedRespawnRemainingTime()
{
}

void AINSPlayerStateBase::AddKill(const int32 KillNum /*= 1*/)
{
	if (HasAuthority())
	{
		Kills += KillNum;
		UpdateKDRatio();
	}
}

void AINSPlayerStateBase::AddMissTakeKill(const int32 KillsToAdd /*= 1*/)
{
	if (HasAuthority())
	{
		MissTakeKill += MissTakeKill;
		UpdateKDRatio();
	}
}

void AINSPlayerStateBase::AddDeath(const int32 DeathToAdd /*= 1*/)
{
	if (HasAuthority())
	{
		Deaths += 1;
		UpdateKDRatio();
	}
}

void AINSPlayerStateBase::UpdateKDRatio()
{
	if (Deaths == 0)
	{
		KDRatio = Kills;
	}
	else
	{
		KDRatio = Kills / Deaths;
	}
	UE_LOG(LogINSPlayerState, Log, TEXT("update k/d ration for player %s,updated k/d ration:%s"), *GetName(), *FString::SanitizeFloat(KDRatio));
}


void AINSPlayerStateBase::AddScore(const float ScoreToAdd)
{
	SetScore(GetScore() + ScoreToAdd);
}


void AINSPlayerStateBase::TickRespawnTime()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (bIsWaitingForRespawn)
		{
			RespawnRemainingTime -= 1.f;
			ReplicatedRespawnRemainingTime = static_cast<uint8>(FMath::CeilToInt(RespawnRemainingTime));
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
	if (GetLocalRole() == ROLE_AutonomousProxy || IsNetMode(NM_ListenServer) || IsNetMode(NM_Standalone))
	{
		if (GetWorld()->GetTimeSeconds() - ComboKillInfo.LastKillTime > ComboKillInfo.ComboBreakTime)
		{
			ComboKillInfo.ComboBreakTime = 0;
		}
	}
}

void AINSPlayerStateBase::OnPawnCharDeath()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsWaitingForRespawn = true;
		const AINSGameStateBase* CurrentGameState = Cast<AINSGameStateBase>(GetWorld()->GetGameState());
		RespawnRemainingTime = CurrentGameState->GetRespawnTime();
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &AINSPlayerStateBase::TickRespawnTime, 1.f, true, 0.f);
	}
}
