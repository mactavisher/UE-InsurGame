// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSCharacter/INSCharacter.h"
#include "INSZombie.generated.h"

class UBehaviorTree;
class UINSZombieAnimInstance;
class AINSZombieController;
class UINSZombieAnimInstance;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogZombiePawn, Log, All);


USTRUCT(BlueprintType)
struct FAttackAnimMontages
{
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
		UAnimMontage* LeftHandAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
		UAnimMontage* RighHandAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
		UAnimMontage* HyperAttackMontage;
};

/**
 * enum specify in which way the zombie zombie will move
 */
UENUM(BlueprintType)
enum class EZombieMoveMode :uint8
{
	Walk                 UMETA(DisplayName = "Walk"),
	Shamble              UMETA(DisplayName = "Shamble"),
	Chase                UMETA(DisplayName = "Chase"),
};

/**
 * enum specify in which way the zombie zombie will move
 */
UENUM(BlueprintType)
enum class EZombieAttackMode :uint8
{
	LeftHand                 UMETA(DisplayName = "LeftHand"),
	RightHand              UMETA(DisplayName = "RightHand"),
	Hyper                UMETA(DisplayName = "Hyper"),
};

/**
 *  zombie character class
 */
UCLASS(notplaceable)
class INSURGENCY_API AINSZombie : public AINSCharacter
{
	GENERATED_UCLASS_BODY()

protected:

	/** zombie behavior tree asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ZombieBehavior")
		UBehaviorTree* ZombiebehaviorTree;

	/** zombie current move mode  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_ZombieMoveMode, Category = "ZombieMovement")
		EZombieMoveMode CurrentZombieMoveMode;

	/** cached zombie controller for this zombie */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
		AINSZombieController* ZombieController;

	/** indicate the zombie attack mode,replicated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_ZombieAttackMode, Category = "Attack")
		EZombieAttackMode CurrenAttackMode;

	/** zombie attack montages */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_ZombieAttackMode, Category = "Attack")
		FAttackAnimMontages ZombieAttackMontages;

	/** cached zombie AnimInstance */
	UPROPERTY()
		UINSZombieAnimInstance* CachedZombieAnimInstance;

protected:

	/**
	 * override
	 */
	virtual void OnRep_Dead()override;

	/**
	 * Get Replicated Properties for net work system
	 * @param OutLifetimeProps  OutLifetimeProps
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	/**
	 * happens when possessed by a zombie controller
	 * @param NewController New Controller that possess this zombie
	 */
	virtual void PossessedBy(AController* NewController)override;

	/**
	 * override
	 */
	virtual void OnRep_LastHitInfo()override;

	/**
	 * face this zombie to a given rotation
	 * @param NewControlRotation Rotation to face to
	 * @DeltaTime DeltaTime
	 */
	virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime /* = 0.f */)override;

	/**
	 * call back function when zombie has changed it's move mode
	 * when it replicate to other clients
	 */
	UFUNCTION()
		virtual void OnRep_ZombieMoveMode();

	UFUNCTION()
		virtual void OnRep_ZombieAttackMode();

public:

	/**
	 * Get the zombie behavior tree asset
	 */
	FORCEINLINE class UBehaviorTree* GetZombieBehaviorTree()const { return ZombiebehaviorTree; }

	/**
	 * Set the zombie current move mode
	 * @param NewZombieMoveMode  the new zombie move mode to set for this zombie
	 */
	virtual void SetZombieMoveMode(EZombieMoveMode NewZombieMoveMode) { CurrentZombieMoveMode = NewZombieMoveMode; };

	/**
	 * Get the zombie current move mode
	 */
	virtual EZombieMoveMode GetZombieMoveMode()const { return CurrentZombieMoveMode; }

	/**
	 * overridden from base character
	 * zombie will have some behavior to set up when they takes some damage
	 */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)override;

	/**
	 * Handles zombie attack request from zombie controller
	 * @param ZombieController From which zombie controller that the attack request is passed
	 */
	virtual void HandlesAttackRequest(class AINSZombieController* ZombieController);

	/**
	 * Get the current zombie attack mode
	 */
	inline virtual EZombieAttackMode GetZombieCurrentAttackMode()const { return CurrenAttackMode; };

	/**
	 * Set the current zombie attack mode
	 * @param NewAttakMode New Attack Mode to set for this zombie
	 */
	virtual void SetZombieCurrentAttackMode(EZombieAttackMode NewAttakMode) { this->CurrenAttackMode = NewAttakMode; };
};
