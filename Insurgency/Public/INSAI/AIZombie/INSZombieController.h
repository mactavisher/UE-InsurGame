// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "INSZombieController.generated.h"


class AINSZombie;
class UBehaviorTreeComponent;
class UPawnSensingComponent;
class UBoxComponent;


INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogZombieController, Log, All);

/**
 *  A zombie controller that responsible for controlling the zombie behavior
 */
UCLASS()
class INSURGENCY_API AINSZombieController : public AAIController
{
	GENERATED_UCLASS_BODY()
protected:
	/** the current acknowledged enemy target */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	AController* CurrentTargetEnemy;

	/** our ai will trying to broad cast enemy to others with this range */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float BroadCastEnemyRange;

	/** our ai will trying to broad cast enemy to others with this range */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float LostEnemyTime;

	/** BehaviorTree comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BehaviorTreeComp", meta = (AllowPrivateAccess = "true"))
	UBehaviorTreeComponent* BehaviorTreeComponent;

	/** BehaviorTree comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ZombieSensingComp", meta = (AllowPrivateAccess = "true"))
	UPawnSensingComponent* ZombieSensingComp;

	/** AttackRange Comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AttackRangeComp", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* AttackRangeComp;

	/** stimulate level for current possessed zombie  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sensing")
	float StimulateLevel;

	/** stimulate location */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sensing")
	FVector StimulateLocation;


	/*
	#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		/** indicates if need to show a line of sight line */
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Debug")
	//uint8 bDrawDebugLineOfSightLine : 1;
	//#endif
	//*/

	/** timer handle for zombies to lost a target enemy if the enemy can't seen for a certain time */
	UPROPERTY()
	FTimerHandle LostEnemyTimerHandle;

protected:
	/**
	 * tick this actor
	 * @param DeltaTime         Tick interval
	 * @param TickType          TickType
	 * @param ThisTickFunction  ThisTickFunction
	 */
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	/**
	 * broad cast enemy to others
	 */
	virtual void BroadCastEnemyTo();

	virtual void InitPlayerState() override;

	/**
	 * Receive broad casted enemy from other
	 * @param InstigatorZombie Player who broadCast this Enemy
	 * @Param Enemy
	 */
	virtual void ReceiveBroadCastedEnemy(class AAIController* InstigatorZombie, AController* Enemy);

	/** returns the current focus enemy*/
	virtual AController* GetMyTargetEnemy() const { return CurrentTargetEnemy; }

	/**
	 * @desc   set the current target enemy,whether this will set is not sure
	 * @param  NewEnemyTarget the new enemy target to set
	 * @return returns true if the Target enemy Set successfully,false otherwise
	 */
	virtual bool TrySetTargetEnemy(class AController* NewEnemyTarget);

	/**
	 * Get The Current zombie that controlled by this zombie controller
	 * @return ZombiePawn
	 */
	inline virtual AINSZombie* GetZombiePawn();

	/**
	 * override when this zombie controller possess it's zombie pawn happened,allow some logic to set up here
	 * @params InPawn the zombie pawn that possessed by this zombie controller
	 */
	virtual void OnPossess(APawn* InPawn) override;

	/**
	 * override when this zombie controller UnPossess it's zombie pawn happened,allow some logic to set up here
	 */
	virtual void OnUnPossess() override;

	/**
	 * randomly initialize a zombies move mode
	 */
	virtual void DetermineZombieMoveMode();

	/**
	 * update the zombie's focal point
	 */
	virtual void UpdateFocalPoint();

	/**
	 * update controller  Rotation
	 * @param DeltaTime   DeltaTime
	 * @param bUpdatePawn indicates if should set pawn's rotation sync with controller rotation
	 */
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn /* = true */) override;


	/**
	 * on see a pawn,call back function bind for PawnSensing comp
	 * @param SeenPawn We see
	 */
	UFUNCTION()
	virtual void OnSeePawn(APawn* SeenPawn);

	/**
	 * on hear noise,call back function bind for PawnSensing comp
	 * @param NoiseInstigator  pawn that made that noise
	 * @param Location    The noise location
	 * @param Volume      The noise volume
	 */
	UFUNCTION()
	virtual void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

	/**
	 * call back function for enemy lost
	 */
	UFUNCTION()
	virtual void OnEnemyLost();

	/**
	 * tick enemy is visible for me
	 */
	UFUNCTION()
	virtual void TickEnemyVisibility();

	/**
	 * performs attack
	 */
	UFUNCTION()
	virtual void ZombieAttack();

	/**
	 * Call back when attack range comp overlapped with something
	 * @param OverlappedComponent my overlapped comp
	 * @param OtherActor Overlapped Actor
	 * @param OtherComp other comp overlapped with this comp
	 * @param bFromSweep is Sweep
	 * @param SweepResult Sweep Result
	 */
	UFUNCTION()
	virtual void OnAttackRangeCompOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


public:
	/**
	 * Add the stimulate to this zombie
	 * @Params ValueToAdd  Value of stimulate to Add
	 */
	virtual void AddStimulate(const float ValueToAdd);

	/**
	 * @Desc Happens when zombie takes damage
	 * @param Damage  damage taken
	 * @param DamageEventInstigator instigator who instigate this damage
	 * @param DamageCausedBy Actor who actually cause this damage
	 */
	virtual void OnZombieTakeDamage(const float Damage, class AController* DamageEventInstigator, class AActor* DamageCausedBy);

	virtual void OnZombieDead();

	/*
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	/**
	 * draw a debug line to provide lines of sight visual
	 */
	//virtual void DrawLOSDebugLine();
	//#endif
	//	*/
};
