// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSProjectileBase.h"
#include "INSProjectile_Server.generated.h"

class USphereComponent;
class AINSWeaponBase;
class AINSProjectile_Visual;
class UINSProjectileMovementComponent;
class UCurveFloat;

/** replicate the projectile hit info explicitly*/
USTRUCT()
struct FRepINSServerProjHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector_NetQuantize LinearVelocity;

	UPROPERTY()
	FVector_NetQuantize10 Location;

	UPROPERTY()
	FRotator Rotation;

	FRepINSServerProjHitInfo()
		: LinearVelocity(ForceInit)
		  , Location(ForceInit)
		  , Rotation(ForceInit)
	{
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		bool bOutSuccessLocal = true;
		// update location, linear velocity
		Location.NetSerialize(Ar, Map, bOutSuccessLocal);
		bOutSuccess &= bOutSuccessLocal;
		Rotation.SerializeCompressed(Ar);
		LinearVelocity.NetSerialize(Ar, Map, bOutSuccessLocal);
		bOutSuccess &= bOutSuccessLocal;
		return true;
	}

	bool operator==(const FRepINSServerProjHitInfo& Other) const
	{
		if (LinearVelocity != Other.LinearVelocity)
		{
			return false;
		}

		if (Location != Other.Location)
		{
			return false;
		}

		if (Rotation != Other.Rotation)
		{
			return false;
		}
		return true;
	}

	bool operator!=(const FRepINSServerProjHitInfo& Other) const
	{
		return !(*this == Other);
	}
};

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogServerProjectile, Log, All);

UCLASS()
class INSURGENCY_API AINSProjectile_Server : public AINSProjectileBase
{
	GENERATED_UCLASS_BODY()
protected:
	/** damage scale by flying distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	UCurveFloat* DamageFalloffCurve;

	/** base damage of this projectile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	float DamageBase;

	/** indicates that this projectile actually hits something , and replicated to notify any other clients*/
	UPROPERTY(VisibleAnywhere, Replicated, ReplicatedUsing = OnRep_HitCounter, BlueprintReadOnly, Category = "Repliciation")
	uint8 HitCounter;

	/** indicates that this projectile actually hits something , and replicated to notify any other clients*/
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool bIsProcessingHit;

	/** how much time to wait before next movement replication happens,used for band width saving purpose*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Repliciation")
	float MovementRepInterval;

	/**record last time we replicate this projectile movement  */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere, Category = "Repliciation")
	float LastMovementRepTime;

	/** indicates how many times dose the net authority projectile Current penetrates*/
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly, Category = "Penetration")
	uint8 CurrentPenetrateCount;

	/** indicates maximum times will the first net authority projectile spawned by a weapon penetrates*/
	UPROPERTY(VisibleAnywhere, Replicated, ReplicatedUsing = OnRep_HitCounter, BlueprintReadOnly, Category = "Penetration")
	uint8 PenetrateCountThreshold;

	/** is this projectile is explosive such as frag */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectileType")
	uint8 bIsExplosive : 1;

	/** due to unavoidable net working delay,we need a little more time for projectile to catch up with net authority version projectile and check impact hit */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	uint8 bDelayedHit : 1;

	/** when movement replication received, check if need a position sync to match server projectile*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	uint8 bNeedPositionSync : 1;

	/** indicates if position sync is in progress */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	uint8 bInPositionSync : 1;

	/** config the velocity and location quantize level,replicated to clients so they know how to unpack the received data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Replication")
	EVectorQuantization MovementQuantizeLevel;

	// UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Visual")
	// TSubclassOf<AINSProjectile_Visual> VisualProjectileClass;

	UPROPERTY()
	AINSProjectile_Visual* VisualProjectile;

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_Hit)
	uint8 bHit:1;

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_ScanTraceHitLoc)
	FVector_NetQuantize10 ScanTraceHitLoc;

	/** ticks to check if client projectile should get created*/
	FActorTickFunction CreateClientVisualProjectileTick;

	/** this used to send a initial replication to relevant client immediately if projectile flies fast */
	UPROPERTY()
	FActorTickFunction InitRepTickFunc;

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_RepHitInfo)
	FRepINSServerProjHitInfo RepHitInfo;


protected:
	//~Begin AActor interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostNetReceiveLocationAndRotation() override;
	virtual void GatherCurrentMovement() override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual void Tick(float DeltaTime) override;
	//~Begin AActor interface
protected:
	virtual void RegisterClientVisualProjectileTick(bool bDoRegister);
	virtual void RegisterInitialTick(bool bDoRegister);
	virtual void UpdateCollisionSettings();
	virtual void CreateVisualProjectile();
	virtual void SendInitialReplication();
	virtual void NotifyVisualProjectileHit();

	//~ begin Replication notify
	UFUNCTION()
	virtual void OnRep_HitCounter();
	virtual void OnRep_ScanTraceCondition() override;
	virtual void OnRep_OwnerWeapon() override;
	virtual void OnRep_ScanTraceTime() override;
	UFUNCTION()
	virtual void OnRep_Hit();
	UFUNCTION()
	virtual void OnRep_ScanTraceHitLoc();
	UFUNCTION()
	virtual void OnRep_RepHitInfo();
	//~ end Replication notify

	//~Begin AINSProjectileBase interface
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	//~End AINSProjectileBase interface

public:
	virtual void SetHitLocation(FVector_NetQuantize10 NewHitLoc);
	virtual void SetScanTraceTime(const float NewTime) override;
	virtual void SetIsScanTraceProjectile(const bool bScanTrace) override;
};
