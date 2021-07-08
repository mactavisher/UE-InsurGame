// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAI/AIZombie/INSZombieController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "INSAI/AIZombie/INSZombie.h"
#include "Components/BoxComponent.h"
#include "INSProjectiles/INSProjectile.h"
#include "INSCharacter/INSPlayerController.h"
#ifndef AINSPlayerCharacter
#include "INSCharacter/INSPlayerCharacter.h"
#endif
#ifndef UCapsuleComponent
#include "Components/CapsuleComponent.h"
#endif
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
	BrainComponent = BehaviorTreeComponent = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviourTreeComp"));
	ZombieSensingComp = ObjectInitializer.CreateDefaultSubobject<UPawnSensingComponent>(this, TEXT("ZombieSensingComp"));
	PrimaryActorTick.bCanEverTick = true;
	AttackRangeComp = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("AttackRangeComp"));
	AttackRangeComp->SetBoxExtent(FVector(200.f, 100.f, 100.f));
	AttackRangeComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StimulateLevel = 0.f;
	StimulateLocation = FVector(ForceInit);
	LostEnemyTime = 5.f;
//#if WITH_EDITOR&&!UE_BUILD_SHIPPING
//	bDrawDebugLineOfSightLine = true;
//	AttackRangeComp->bHiddenInGame = false;
//#endif
//#if !WITH_EDITOR&&UE_BUILD_SHIPPING
//	AttackRangeComp->bHiddenInGame = true;
//#endif
}


void AINSZombieController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (CurrentTargetEnemy)
	{
		TickEnemyVisibility();
		StimulateLocation = CurrentTargetEnemy->GetPawn()->GetActorLocation();
	}
	if (!StimulateLocation.IsZero())
	{
		SetFocalPoint(StimulateLocation);
	}
	if (CurrentTargetEnemy)
	{
		MoveToActor(CurrentTargetEnemy->GetPawn());
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
		StimulateLocation = NewEnemyTarget->GetPawn()->GetActorLocation();
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
			ZombieSensingComp->OnSeePawn.AddDynamic(this, &AINSZombieController::OnSeePawn);
			ZombieSensingComp->OnHearNoise.AddDynamic(this, &AINSZombieController::OnHearNoise);
			AttackRangeComp->OnComponentBeginOverlap.AddDynamic(this, &AINSZombieController::OnAttackRangeCompOverlap);
			AttackRangeComp->AttachToComponent(GetZombiePawn()->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			AttackRangeComp->SetRelativeLocation(FVector(50.f, 0.f, 0.f));
			AttackRangeComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		RunBehaviorTree(GetZombiePawn()->GetZombieBehaviorTree());
	}
}

void AINSZombieController::OnUnPossess()
{
	Super::OnUnPossess();
	if (BehaviorTreeComponent && BehaviorTreeComponent->IsRunning())
	{
		ZombieSensingComp->OnSeePawn.RemoveAll(this);
		ZombieSensingComp->OnHearNoise.RemoveAll(this);
		BehaviorTreeComponent->StopTree(EBTStopMode::Safe);
		AttackRangeComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		AttackRangeComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

void AINSZombieController::UpdateFocalPoint()
{

}

void AINSZombieController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn /* = true */)
{
	Super::UpdateControlRotation(DeltaTime, bUpdatePawn);
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
	else
	{
		TrySetTargetEnemy(SeenPawn->GetController());
	}
}

void AINSZombieController::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{

}

void AINSZombieController::OnEnemyLost()
{
	CurrentTargetEnemy = nullptr;
	StimulateLocation = FVector(ForceInit);
	GetWorldTimerManager().ClearTimer(LostEnemyTimerHandle);
	UE_LOG(LogZombieController
		, Log
		, TEXT("%s Zombie has lost target enemy")
		, *GetZombiePawn()->GetName());
}

void AINSZombieController::TickEnemyVisibility()
{
	if (CurrentTargetEnemy)
	{
		bool isEnemyVisible = LineOfSightTo(GetMyTargetEnemy()->GetPawn());
		if (!isEnemyVisible)
		{
			if (!GetWorldTimerManager().IsTimerActive(LostEnemyTimerHandle))
			{
				GetWorldTimerManager().SetTimer(LostEnemyTimerHandle, this, &AINSZombieController::OnEnemyLost, 1.f, false, LostEnemyTime);
			}
		}
//#if WITH_EDITOR&&!UE_BUILD_SHIPPING
//		DrawLOSDebugLine();
//#endif
	}
}

void AINSZombieController::ZombieAttack()
{
	if (GetZombiePawn()&&!GetZombiePawn()->GetIsDead())
	{
		
	}
}

void AINSZombieController::OnAttackRangeCompOverlap(UPrimitiveComponent* OverlappedComponent
	, AActor* OtherActor
	, UPrimitiveComponent* OtherComp
	, int32 OtherBodyIndex
	, bool bFromSweep
	, const FHitResult& SweepResult)
{
	//refer to face to BB-entry
	if (OtherActor->GetClass()->IsChildOf(AINSPlayerCharacter::StaticClass()))
	{
		if (!GetMyTargetEnemy())
		{
			CurrentTargetEnemy = Cast<AController>(OtherActor->GetOwner());
		}
	}
	else if (GetMyTargetEnemy() == OtherActor->GetOwner())
	{

	}
}

void AINSZombieController::AddStimulate(float ValueToAdd)
{
	this->StimulateLevel += ValueToAdd;
	UE_LOG(LogZombieController
		, Log
		, TEXT("%s Zombie has increase Stimulate %f!")
		, *GetZombiePawn()->GetName()
		, ValueToAdd);
}

void AINSZombieController::OnZombieTakeDamage(float Damage, class AController* DamageEventInstigator, class AActor* DamageCausedBy)
{
	if (Damage <= 0.f)
	{
		return;
	}
	else
	{
		if (DamageCausedBy)
		{
			const UClass* const DamageEventInstigatorClass = DamageEventInstigator->GetClass();
			if (DamageEventInstigatorClass->IsChildOf(AINSPlayerController::StaticClass()))
			{
				StimulateLocation = DamageEventInstigator->GetPawn()->GetActorLocation();
				AddStimulate(Damage);
			}
		}
	}
}

//#if WITH_EDITOR && !UE_BUILD_SHIPPING
//void AINSZombieController::DrawLOSDebugLine()
//{
//	if (bDrawDebugLineOfSightLine)
//	{
//		FRotator ViewRotation;
//		FVector ViewPoint;
//		GetActorEyesViewPoint(ViewPoint, ViewRotation);
//		DrawDebugLine(GetWorld(), ViewPoint, GetMyTargetEnemy()->GetPawn()->GetActorLocation(), FColor::Green, false, 0.5f);
//	}
//}
//#endif

