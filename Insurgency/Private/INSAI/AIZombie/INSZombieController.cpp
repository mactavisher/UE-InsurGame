// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAI/AIZombie/INSZombieController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif
#include "INSAI/AIZombie/INSZombie.h"
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
	PrimaryActorTick.bCanEverTick = true;
}


void AINSZombieController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (CurrentTargetEnemy)
	{
		LineOfSightTo(CurrentTargetEnemy);
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		DrawLOSDebugLine();
#endif
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
		RunBehaviorTree(GetZombiePawn()->GetZombieBehaviorTree());
	}
}

void AINSZombieController::OnUnPossess()
{
	Super::OnUnPossess();
	if (BehaviorTreeComponent&&BehaviorTreeComponent->IsRunning())
	{
		BehaviorTreeComponent->StopTree(EBTStopMode::Safe);
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

