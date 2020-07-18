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

AINSPlayerStateBase::AINSPlayerStateBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bIsWaitingForRespawn = false;
	RespawnRemainingTime = 0.f;
	CachedDamageInfoMaxSize = 5;
	CachedDamageInfos.SetNum(CachedDamageInfoMaxSize);
}
void AINSPlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSPlayerStateBase, PlayerTeam);
	DOREPLIFETIME(AINSPlayerStateBase, MyScore);
	DOREPLIFETIME(AINSPlayerStateBase, bIsWaitingForRespawn);
	DOREPLIFETIME(AINSPlayerStateBase, ReplicatedRespawnRemainingTime);
}

void AINSPlayerStateBase::OnRep_Score()
{
	if (GetPawn()&& GetPawn()->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::FromInt(Score));
	}
}

void AINSPlayerStateBase::OnRep_TeamInfo()
{
	/*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Team info Replicated"));
	AINSPlayerCharacter* const PlayerCharacter = GetPawn<AINSPlayerCharacter>();
	if (PlayerCharacter)
	{
		PlayerCharacter->SetupPlayerMesh();
	}*/
}

void AINSPlayerStateBase::OnRep_MyScore()
{
	//if (GetPawn()&& GetPawn()->IsLocallyControlled())
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::FromInt(MyScore));
	//}
}

void AINSPlayerStateBase::OnRep_RespawnRemainingTime()
{

}

void AINSPlayerStateBase::SetPlayerTeam(class AINSTeamInfo* NewTeam)
{
	PlayerTeam = NewTeam;
}

void AINSPlayerStateBase::UpdateRepliatedRespawnRemaingTime()
{

}

void AINSPlayerStateBase::ReceiveHitInfo(const struct FTakeHitInfo TakeHitInfo)
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
