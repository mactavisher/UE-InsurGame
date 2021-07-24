// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSCharacter/INSCharacter.h"
#include "INSZombie.generated.h"

class UBehaviorTree;
class UINSZombieAnimInstance;
class AINSZombieController;
class UINSZombieAnimInstance;
class USkeletalMeshComponent;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogZombiePawn, Log, All);


USTRUCT(BlueprintType)
struct FAttackAnimMontages
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* LeftHandAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* RightHandAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* HyperAttackMontage;
};

/**
 * enum specify in which way the zombie zombie will move
 */
UENUM(BlueprintType)
enum class EZombieMoveMode :uint8
{
	Walk UMETA(DisplayName = "Walk"),
	Shamble UMETA(DisplayName = "Shamble"),
	Chase UMETA(DisplayName = "Chase"),
};

/**
 * enum specify in which way the zombie zombie will move
 */
UENUM(BlueprintType)
enum class EZombieAttackMode :uint8
{
	LeftHand UMETA(DisplayName = "Attack Using Left Hand"),
	RightHand UMETA(DisplayName = "Attack Using Right Hand"),
	Hyper UMETA(DisplayName = "Hyper Attack"),
};

/**
 *  zombie character class
 */
UCLASS(Blueprintable)
class INSURGENCY_API AINSZombie : public AINSCharacter
{
	GENERATED_UCLASS_BODY()
protected:
	/** zombie behavior tree asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ZombieBehavior")
	UBehaviorTree* ZombieBehaviorTree;

	/** zombie current move mode  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_ZombieMoveMode,
		Category = "ZombieMovement")
	EZombieMoveMode CurrentZombieMoveMode;

	/** cached zombie controller for this zombie */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	AINSZombieController* ZombieController;

	/** indicate the zombie attack mode,replicated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_ZombieAttackMode,
		Category = "Attack")
	EZombieAttackMode CurrenAttackMode;

	/** zombie attack montages */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	FAttackAnimMontages ZombieAttackMontages;

	/** cached zombie AnimInstance */
	UPROPERTY()
	UINSZombieAnimInstance* CachedZombieAnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float AttackDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rage")
	float RagePoint;

	/** modular zombie of HeadComp */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HeadComp;

	/** modular zombie of TorsoComp */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* TorsoComp;

	/** modular zombie of LeftArmComp */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LeftArmComp;

	/** modular zombie of RightArmComp */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* RightArmComp;

	/** modular zombie of LeftLegComp */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LeftLegComp;

	/** modular zombie of rightLegComp */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* RightLegComp;

	/** cache the modular skeletal meshes for later easy access purpose */
	UPROPERTY()
	TArray<USkeletalMeshComponent*> CachedModularSkeletalMeshes;

protected:
	/**
	 * override
	 */
	virtual void OnRep_Dead() override;

	/**
	 * Get Replicated Properties for net work system
	 * @param OutLifetimeProps  OutLifetimeProps
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * happens when possessed by a zombie controller
	 * @param NewController New Controller that possess this zombie
	 */
	virtual void PossessedBy(AController* NewController) override;

	virtual void PostInitializeComponents() override;

	/**
	 * override
	 */
	virtual void OnRep_LastHitInfo() override;

	/**
	 * face this zombie to a given rotation
	 * @param NewControlRotation Rotation to face to4
	 * @Param DeltaTime the world delta tme in seconds
	 */
	virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime /* = 0.f */) override;

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
	FORCEINLINE class UBehaviorTree* GetZombieBehaviorTree() const { return ZombieBehaviorTree; }

	/**
	 * Set the zombie current move mode
	 * @param NewZombieMoveMode  the new zombie move mode to set for this zombie
	 */
	virtual void SetZombieMoveMode(EZombieMoveMode NewZombieMoveMode) { CurrentZombieMoveMode = NewZombieMoveMode; };

	/**
	 * Get the zombie current move mode
	 */
	virtual EZombieMoveMode GetZombieMoveMode() const { return CurrentZombieMoveMode; }

	/**
	 * overridden from base character
	 * zombie will have some behavior to set up when they takes some damage
	 */
	virtual float TakeDamage(const float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;

	/**
	 * Handles zombie attack request from zombie controller
	 * @param RequestZombieController From which zombie controller that the attack request is passed
	 */
	virtual void HandlesAttackRequest(class AINSZombieController* RequestZombieController);

	/**
	 * Get the current zombie attack mode
	 */
	inline virtual EZombieAttackMode GetZombieCurrentAttackMode() const { return CurrenAttackMode; };

	/**
	 * Set the current zombie attack mode
	 * @param NewAttackMode New Attack Mode to set for this zombie
	 */
	virtual void SetZombieCurrentAttackMode(const enum EZombieAttackMode NewAttackMode) { this->CurrenAttackMode = NewAttackMode; };

	/**
	 * performs a line trace damage
	 */
	virtual void PerformLineTraceDamage();

	/**
	 * retrieves the zombie's current rage point
	 */
	inline virtual float GetZombieRagePoint() { return RagePoint; };

	/**
	 * add rage point to  zombie's current rage point
	 */
	virtual void AddZombieRagePoint(const int32 RageToAdd) { RagePoint += RageToAdd; }
};
