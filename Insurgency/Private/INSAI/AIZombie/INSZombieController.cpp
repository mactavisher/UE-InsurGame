// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAI/AIZombie/INSZombieController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "INSAI/AIZombie/INSZombie.h"
#ifndef FTimerManager
#include "TimerManager.h"
#endif
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif
#ifndef UEngineUtils
#include "EngineUtils.h"
#endif

DEFINE_LOG_CATEGORY(LogZombieController);
AINSZombieController::AINSZombieController(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	BroadCastEnemyRange = 10000.f;
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	bDrawDebugLineOfSightLine = true;
#endif
	BrainComponent = BehaviorTreeComponent = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviourTreeComp"));
	ZombieSensingComp = ObjectInitializer.CreateDefaultSubobject<UPawnSensingComponent>(this, TEXT("ZombieSensingComp"));
	PrimaryActorTick.bCanEverTick = true;
}


void AINSZombieController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (CurrentTargetEnemy)
	{
		TickEnemyVisibility();
	}
}

void AINSZombieController::BroadCastEnemyTo()
{
	UWorld* const CurrentWorld = GetWorld();

	for (TActorIterator<AINSZombieController> It(CurrentWorld); It; ++It)
	{
		const AINSZombieController* const ZombieController = *It;
		if (ZombieController)
		{
			AController* ThatZombieTarget = ZombieController->GetMyTargetEnemy();
			if (ThatZombieTarget == nullptr)
			{
				TrySetTargetEnemy(GetMyTargetEnemy());
			}
			else if (ThatZombieTarget == GetMyTargetEnemy())
			{
				UE_LOG(LogZombieController
					, Log
					, TEXT("Zombie %s and I have the same enemy target %s,abort seting target enemy")
					, ZombieController == nullptr ? TEXT("NULL") : *ZombieController->GetName()
					, *GetName())
					return;
			}
		}
	}
}

void AINSZombieController::ReceiveBroadCastedEnemy(class AAIController* InstigatorZombie, AController* Enemey)
{

}

bool AINSZombieController::TrySetTargetEnemy(class AController* NewEnemyTarget)
{
	const int32 RandomNum = FMath::RandHelper(10);
	if (RandomNum % 3 == 0)
	{
		CurrentTargetEnemy = NewEnemyTarget;
		if (GetWorldTimerManager().IsTimerActive(LostEnemyTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(LostEnemyTimerHandle);
		}
		return true;
	}
	else
	{
		UE_LOG(LogZombieController
			, Log
			, TEXT("%s trying to set it's enemy target , but %s Decide to do nothing!")
			, *GetName()
			, *GetName());
		return false;
	}
}

void AINSZombieController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (InPawn)
	{
		InitZombieMoveMode();
		if (ZombieSensingComp)
		{
			// bind delegate
			ZombieSensingComp->OnSeePawn.AddDynamic(this, &AINSZombieController::OnSeePawn);
			ZombieSensingComp->OnHearNoise.AddDynamic(this, &AINSZombieController::OnHearNoise);
		}
		RunBehaviorTree(GetZombiePawn()->GetZombieBehaviorTree());
	}
}

void AINSZombieController::OnUnPossess()
{
	Super::OnUnPossess();
	if (BehaviorTreeComponent&&BehaviorTreeComponent->IsRunning())
	{
		ZombieSensingComp->OnSeePawn.RemoveAll(this);
		ZombieSensingComp->OnHearNoise.RemoveAll(this);
		BehaviorTreeComponent->StopTree(EBTStopMode::Safe);
	}
}

void AINSZombieController::InitZombieMoveMode()
{
	const int32 RandomNum = FMath::RandHelper(2);
	EZombieMoveMode SelectedZombieMoveMode;
	switch (RandomNum)
	{
	case 0:SelectedZombieMoveMode = EZombieMoveMode::Shamble; break;
	case 1:SelectedZombieMoveMode = EZombieMoveMode::Walk; break;
	case 2:SelectedZombieMoveMode = EZombieMoveMode::Chase; break;
	default:SelectedZombieMoveMode = EZombieMoveMode::Shamble; break;
	}
	GetZombiePawn()->SetZombieMoveMode(SelectedZombieMoveMode);
}

void AINSZombieController::OnSeePawn(APawn* SeenPawn)
{
	if (SeenPawn->GetClass()->IsChildOf(AINSZombie::StaticClass()))
	{
		// we are zombies 
		UE_LOG(LogZombieController
			, Log
			, TEXT("%s is another zombie of my type ,can't set it as my enemy !")
			, *SeenPawn->GetName());
		return;
	}
	if (GetMyTargetEnemy())
	{
		if (GetMyTargetEnemy()->GetPawn() == SeenPawn)
		{
			// that pawn is already my enemy now 
			UE_LOG(LogZombieController
				, Log
				, TEXT("%s is is already my enemy now !")
				, *SeenPawn->GetName());
			return;
		}
		else
		{
			TrySetTargetEnemy(SeenPawn->GetController());
		}
	}
}

void AINSZombieController::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{

}

void AINSZombieController::OnEnemyLost()
{
	CurrentTargetEnemy = nullptr;
	GetWorldTimerManager().ClearTimer(LostEnemyTimerHandle);
}

void AINSZombieController::TickEnemyVisibility()
{
	if (CurrentTargetEnemy)
	{
		const bool isEnemyVisible = LineOfSightTo(CurrentTargetEnemy);
		if (!isEnemyVisible)
		{
			GetWorldTimerManager().SetTimer(LostEnemyTimerHandle, this, &AINSZombieController::OnEnemyLost, 1.f, false, LostEnemyTime);
		}
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		DrawLOSDebugLine();
#endif
	}
}

#if WITH_EDITOR && !UE_BUILD_SHIPPING
void AINSZombieController::DrawLOSDebugLine()
{
	if (bDrawDebugLineOfSightLine)
	{
		FRotator ViewRotation;
		FVector ViewPoint;
		GetActorEyesViewPoint(ViewPoint, ViewRotation);
		DrawDebugLine(GetWorld(), ViewPoint, CurrentTargetEnemy->GetTargetLocation(GetZombiePawn()), FColor::Green, false, 0.5f);
	}
}
#endif

