// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "INSZombieController.generated.h"


class AINSZombie;
class UBehaviorTreeComponent;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogZombieController, Log, All);

/**
 *  A zombie controller that responsible for controlling the zombie behavior
 */
UCLASS()
class INSURGENCY_API AINSZombieController : public AAIController
{
	GENERATED_UCLASS_BODY()

	/** the current acknowledged enemy target */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
		AController* CurrentTargetEnemy;

	/** our ai will trying to broad cast enemy to others with this range */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
		float BroadCastEnemyRange;

	/** BehaviorTree comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BehaviorTreeComp",meta = (AllowPrivateAccess = "true"))
		class UBehaviorTreeComponent* BehaviorTreeComponent;

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	/** indicates if need to show a line of sight line */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Debug")
		uint8 bDrawDebugLineOfSightLine : 1;
#endif

protected:

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

	/**
	 * broad cast enemy to others
	 */
	virtual void BroadCastEnemyTo();

	/**
	 * Receive broad casted enemy from other
	 * @param Instigator Player who broadCast this Enemey
	 */
	virtual void ReceiveBroadCastedEnemy(class AAIController* InstigatorZombie, AController* Enemey);

	/**
	 * Get The Current Target Enemey
	 * @param Instigator Player who broadCast this Enemey
	 */
	inline virtual AController* GetMyTargetEnemy()const { return CurrentTargetEnemy; }

	/**
	 * @desc   set the current target enemy,whether this will set is not sure
	 * @param  NewTargetEnemy the new enemy target to set
	 * @return returns true if the Target enemy Set successfully,false otherwise
	 */
	virtual bool TrySetTargetEnemy(class AController* NewEnemyTarget);

	/**
	 * Get The Current zombie that controlled by this zombie controller
	 * @return Zombie that controlled by this controller
	 */
	inline virtual AINSZombie* GetZombiePawn() { return GetPawn<AINSZombie>(); }

	/**
	 * override when this zombie controller possess it's zombie pawn happened,allow some logic to set up here
	 * @params InPawn the zombie pawn that possessed by this zombie controller
	 */
	virtual void OnPossess(APawn* InPawn)override;

	/**
 * override when this zombie controller possess it's zombie pawn happened,allow some logic to set up here
 * @params InPawn the zombie pawn that possessed by this zombie controller
 */
	virtual void OnUnPossess()override;

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	/**
	 * draw a debug line to provide lines of sight visual
	 */
	virtual void DrawLOSDebugLine();
#endif
};
