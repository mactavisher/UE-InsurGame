// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "INSProjectile.generated.h"

class AINSWeaponBase;
class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UINSProjectileMovementComponent;
class UParticleSystemComponent;
class AINSImpactEffect;
class UMatineeCameraShake;
class UDamageType;
class UCurveFloat;
class AINSWeaponBase;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSProjectile, Log, All);

//no need to override engine's replicate movement system,since it has be optimized
USTRUCT()
struct FRepINSProjMovement
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		FVector_NetQuantize LinearVelocity;

	UPROPERTY()
		FVector_NetQuantize Location;

	UPROPERTY()
		FRotator Rotation;

	FRepINSProjMovement()
		: LinearVelocity(ForceInit)
		, Location(ForceInit)
		, Rotation(ForceInit)
	{}

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

	bool operator==(const FRepINSProjMovement& Other) const
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

	bool operator!=(const FRepINSProjMovement& Other) const
	{
		return !(*this == Other);
	}
};

USTRUCT(BlueprintType)
struct FProjectleLiftTimeData
{

	GENERATED_USTRUCT_BODY()

		/** indicates the initial speed after this projectile fired, usually be the weapon's muzzle speed */
	UPROPERTY()
		float StartSpeed;

	/** indicates the impact speed after this projectile fired, usually be the weapon's muzzle speed */
	UPROPERTY()
		float ImpactSpeed;

	/** indicates the impact Velocity after this projectile fired, usually be the weapon's muzzle speed */
	UPROPERTY()
		FVector ImpactVelocity;

	/** indicates the actor that impact with */
	UPROPERTY()
		TWeakObjectPtr<AActor> ImpactActor;

	/** indicates the Player that impact with if exist,could be null */
	UPROPERTY()
		TWeakObjectPtr<AController> ImpactPlayer;

	/** indicates the time fires this shot */
	UPROPERTY()
		float StartTime;

	/** indicates the time fires this shot */
	UPROPERTY()
		float ImpactTime;

	UPROPERTY()
		int32 DamageCaused;
};


/**
 * master class of weapon projectiles
 * when in a multi player mode,this projectile will be replicated to
 * all clients and besides,when this projectiles is replicated ,it will
 * spawn a client fake projectile to provide visuals,if use the replicated one
 * the movement of this projectile may be choppy if velocity is not Replicated or
 * quantized,so we can make a client fake projectile and init it just as the server
 * version doses but from where we can optimize movement replication,and sync some
 * data that drives this projectile movement replicated from server
 */
UCLASS(Abstract, Blueprintable)
class INSURGENCY_API AINSProjectile : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	/** collision comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CollisionComp", meta = (AllowPrivateAccess = "true"))
		USphereComponent* CollisionComp;

	/** projectile visual mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MeshComp", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* ProjectileMesh;

	/** projectile movement comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ProjectileMoveMentComp", meta = (AllowPrivateAccess = "true"))
		UINSProjectileMovementComponent* ProjectileMoveComp;

	/** projectile trace effect  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, category = "Effects|Trace", meta = (AllowPrivateAccess = "true"))
		UParticleSystemComponent* TracerParticle;

	/** point impact effect class  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactEffects")
		TSubclassOf<AINSImpactEffect>PointImapactEffectsClass;

	/** radius impact effect class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactEffects")
		TSubclassOf<AINSImpactEffect>ExplodeImapactEffectsClass;

	/** point damage type class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		TSubclassOf<UDamageType> ProjectileDamageTypeClass;

	/** radius damage type class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		TSubclassOf<UDamageType> ExplosiveDamageTypeClass;

	/** damage scale by flying distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		UCurveFloat* DamageFalloffCurve;

	/** base damage of this projectile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float DamageBase;

	/** indicates that this projectile actually hits something , and replicated to notify any other clients*/
	UPROPERTY(VisibleAnywhere, Replicated, ReplicatedUsing = OnRep_HitCounter, BlueprintReadOnly, Category = "Repliciation")
		uint8  HitCounter;

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
		uint8  CurrentPenetrateCount;

	/** indicates maximum times will the first net authority projectile spawned by a weapon penetrates*/
	UPROPERTY(VisibleAnywhere, Replicated, ReplicatedUsing = OnRep_HitCounter, BlueprintReadOnly, Category = "Penetration")
		uint8  PenetrateCountThreshold;

	/** is this projectile is explosive such as frag */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectileType")
		uint8 bIsExplosive : 1;

	/** due to unavoidable net working delay,we need a little more time for projectile to catch up with net authority version projectile and check impact hit */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
		uint8 bDelayedHit : 1;

	/** is this projectile exploded last frame */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_Explode, Category = "State")
		uint8 bExploded : 1;

	/** is a client fake projectile used for providing visuals for client */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
		uint8 bVisualProjectile : 1;

	/** when movement replication received, check if need a position sync to match server projectile*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		uint8 bNeedPositionSync : 1;

	/** indicates if position sync is in progress */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		uint8 bInPositionSync : 1;

	/** is this projectile hit by way of scan trace, if true ,we just need to tell client where it flies and no need movement replication or scale the the movement rep interval */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Replication")
		uint8 bScanTraceProjectile : 1;

	/** config the velocity and location quantize level,replicated to clients so they know how to unpack the received data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Replication")
		EVectorQuantization MovementQuantizeLevel;

	/** Net authority version */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Replication")
		AINSProjectile* NetAuthrotyProjectile;

	/** client fake version ,only provide visual*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Replication")
		AINSProjectile* VisualFakeProjectile;

	/** weapon that fires me */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_OwnerWeapon, Category = "WeaponOwner")
		AINSWeaponBase* OwnerWeapon;

	/** weak ptr type ,player that actually fire this projectile */
	TWeakObjectPtr<AController> InstigatorPlayer;

	/** force a movement info replicate to clients */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Replication")
		uint8 bForceMovementReplication : 1;

	/** indicate that movement replication is in progress*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Replication")
		bool bIsGatheringMovement;

	/** indicate if the scan trace projectile has reach the desire location */
	UPROPERTY()
	uint8 bReachDesiredLoc;

	/** this used to send a initial replication to relevant client immediately if projectile flies fast */
	UPROPERTY()
		FActorTickFunction InitRepTickFunc;

	UPROPERTY()
		FActorTickFunction TracerPaticleSizeTickFun;

	UPROPERTY()
	    FVector SpawnLocation;

	UPROPERTY(Replicated)
	    FVector ScanTraceHitLoc;

	UPROPERTY()
	    float ScanTraceTime;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile|Debug")
		uint8 bUsingDebugTrace : 1;
#endif

private:
	FTimerHandle FakeProjVisibilityTimer;
protected:

	/** notify clients this projectile is exploded */
	UFUNCTION()
		virtual void OnRep_Explode();

	/** notify clients this projectile hit somethings, effect to be handled in clients side */
	UFUNCTION()
		virtual void OnRep_HitCounter();

	/** create a fake projectile when valid owner is replicated */
	UFUNCTION()
		virtual void OnRep_OwnerWeapon();

	/** Enable projectile visual */
	UFUNCTION()
		virtual void EnableFakeProjVisual();

	/** delegate call back for collision detect */
	UFUNCTION()
		virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/*UFUNCTION()
		virtual void CalAndSpawnPenetrateProjectile(const FHitResult& OriginHitResult, const FVector& OriginVelocity);*/
protected:

	// ~Begin AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds)override;
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;
	virtual void PostInitializeComponents()override;
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity)override;
	virtual void PostNetReceiveLocationAndRotation()override;
	virtual void GatherCurrentMovement()override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	virtual void OnRep_ReplicatedMovement()override;
	// ~End AActor interface

	/**
	 * @Desc instantiate a client fake projectile for providing visual effect purpose
	 *       if use the directly replicated Projectile from server ,it's may get choppy
	 *       since we have modify it's replication frequency,so the solution is when the
	 *       server side projectile Replicated and it's owner is Replicated,instantiate the
	 *       client fake projectile immediately and still simulate it's physic state,and by
	 *       each time we received a update ,just update the fake to match the server one
	 */
	virtual void InitClientFakeProjectile(const AINSProjectile* const NetAuthorityProjectile);

	/**
	 * @Desc send a initial replication to relevant clients for extremely fast moving projectiles
	 *       work with InitRepTickFunc
	 */
	virtual void SendInitialReplication();

	/** Checks the Impact hit result and simulate FX */
	virtual void CheckImpactHit();


public:

	/** return base damage amount */
	inline virtual float GetBaseDamage()const { return DamageBase; }

	virtual void SetScantraceHitLoc(const FVector NewLoc) { ScanTraceHitLoc = NewLoc; }

	/** get the current penetrate count */
	inline virtual uint8 GetCurrentPenetrateCount()const { return CurrentPenetrateCount; }

	/** set the current penetrate count */
	virtual void SetCurrentPenetrateCount(uint8 AddInCount = 1) { CurrentPenetrateCount += AddInCount; }

	/** set owner weapon */
	virtual void SetOwnerWeapon(class AINSWeaponBase* NewWeaponOwner);

	/** set instigator player that fires this projectile */
	virtual void SetInstigatedPlayer(class AController* InsigatedPlayer) { InstigatorPlayer = InsigatedPlayer; }

	/** set is fake projectile */
	virtual void SetIsFakeProjectile(bool bFake) { this->bVisualProjectile = bFake; }

	virtual void SetSpawnLocation(const FVector NewSpawnLocation) { SpawnLocation = NewSpawnLocation; }

	virtual FVector GetSpawnLocation()const { return SpawnLocation; }

	/** get is fake projectile */
	inline virtual bool GetIsFakeProjectile()const { return bVisualProjectile; }

	/** get owner weapon */
	virtual class AINSWeaponBase* GetOwnerWeapon()const { return OwnerWeapon; }

	/** get the client fake projectile */
	virtual class AINSProjectile* GetVisualFakeProjectile()const { return VisualFakeProjectile; }

	/** get actual damage taken */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BFProjectile")
		virtual float GetDamageTaken();

	/** return collision comp */
	FORCEINLINE USphereComponent* GetCollsioncomp()const { return CollisionComp; }

	/** return mesh comp */
	FORCEINLINE UStaticMeshComponent* GetProjectileMesh()const { return ProjectileMesh; }

	/** return projectile movement comp */
	FORCEINLINE UINSProjectileMovementComponent* GetProjectileMovementComp()const { return ProjectileMoveComp; }

	/**
	 * @desc  set the initial speed this projectile will use to travel
	 * @Param NewSpeed  The New Speed to set
	 */
	virtual void SetMuzzleSpeed(float NewSpeed);

	/**
	 * @desc set the scan trace time that this projectile will travel,during scan trace time ,projectiles will have no
	 *       physics simulation such as gravity
	 * 
	 * @Param NewTime  the scan trace time to set
	 */
	virtual void SetScanTraceTime(float NewTime);

	/** returns the projectile scan trace time */
	virtual float GetScanTraceTime()const { return ScanTraceTime; }

	/**
	 * @Desc  set if this projectile is spawned by way of scan trace
	 * @Param bFromScanTrace whether this projectile is spawned by way of scan trace or not
	 */
	virtual void SetScanTraceProjectile(bool bFromScanTrace) { bScanTraceProjectile = bFromScanTrace; }

	/**
	 * Set the fake projectile of the net Authority one,Each client will have one of this fake projectile
	 * @param NewFakeProjectile  New Fake Projectile to set
	 */
	virtual void SetClientFakeProjectile(class AINSProjectile* NewFakeProjectile) { VisualFakeProjectile = NewFakeProjectile; };

};
