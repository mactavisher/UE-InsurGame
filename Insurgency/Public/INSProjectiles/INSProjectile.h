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
class UProjectileMovementComponent;
class UParticleSystemComponent;
class AINSImpactEffect;
class UCameraShake;
class UDamageType;
class UCurveFloat;
class AINSWeaponBase;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSProjectile, Log, All);

/** Replicated movement data of our RootComponent.
*   More efficient than engine's FRepMovement
*/
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


/**
 * master class of weapon projectiles
 * when in a multi player mode,this projectile will be replicated to
 * all clients and besides,when this projectiles is replicated ,it will
 * spawn a client fake projectile to provide visuals,if use the replicated one
 * the movement of this projectile may be choppy if velocity is not Replicated or
 * quantized,so we can make a client fake projectile and init it just as the server
 * version doses but from where we can optimize movement replication,and sync some
 * data that drives this projectile movement replicated from server
 *
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

	/** Used for replication of our RootComponent's position and velocity */
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_INSProjReplicatedMovement)
		struct FRepINSProjMovement INSProjReplicatedMovement;

	/** projectile movement comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ProjectileMoveMentComp", meta = (AllowPrivateAccess = "true"))
		UProjectileMovementComponent* ProjectileMoveComp;

	/** projectile trace effect  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, category = "Effects|Trace", meta = (AllowPrivateAccess = "true"))
		UParticleSystemComponent* TracerParticle;

	/** point impact effect class  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects|Impacts")
		TSubclassOf<AINSImpactEffect>PointImapactEffectsClass;

	/** radius impact effect class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects|Impacts")
		TSubclassOf<AINSImpactEffect>ExplodeImapactEffectsClass;

	/** camera shake feed back to player hit by this projectile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects|CameraShake")
		TSubclassOf<UCameraShake> CameraShakeClass;

	/** point damage type class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage|DamageType")
		TSubclassOf<UDamageType> ProjectileDamageTypeClass;

	/** radius damage type class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage|DamageType")
		TSubclassOf<UDamageType> ExplosiveDamageTypeClass;

	/** damage scale by flying distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage|DamageFalloffCurve")
		UCurveFloat* DamageFalloffCurve;

	/** base damage of this projectile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage|Amount")
		float DamageBase;

	/** indicates that this projectile actually hits something , and replicated to notify any other clients*/
	UPROPERTY(VisibleAnywhere, Replicated, ReplicatedUsing = OnRep_HitCounter, BlueprintReadOnly, Category = "Repliciation|Hit")
		uint8  HitCounter;

	/** indicates how many times dose the net authority projectile Current penetrates*/
	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly, Category = "Repliciation|Hit")
		uint8  CurrentPenetrateCount;

	/** indicates maximum times will the net authority projectile penetrates*/
	UPROPERTY(VisibleAnywhere, Replicated, ReplicatedUsing = OnRep_HitCounter, BlueprintReadOnly, Category = "Repliciation|Hit")
		uint8  PenetrateCountThreshold;

	/** hit result when this projectile hit something */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFProjectile|HitResult")
		FHitResult ImpactHit;

	/** is this projectile is explosive such as frags */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFProjectile|Explosive")
		uint8 bIsExplosive : 1;

	/** is this projectile exploded last frame */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_Explode, Category = "Repliciation|Hit")
		uint8 bIsExplode : 1;

	/** is a visual fake projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFProjectile|Explosive")
		uint8 bClientFakeProjectile : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Replication")
		uint8 bRepINSMovement : 1;

	/** is a visual fake projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFProjectile|Explosive")
		uint8 bHideVisualWhenSpawened : 1;

	/** Net authority version */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFProjectile")
		AINSProjectile* NetAuthrotyProjectile;

	/** client fake version ,only provide visual*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFProjectile")
		AINSProjectile* ClientFakeProjectile;

	/** weapon that fires me */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_OwnerWeapon, Category = "WeaponOwner")
		AINSWeaponBase* OwnerWeapon;

	/** weak ptr type ,player that actually fire this projectile */
	TWeakObjectPtr<AController> InstigatorPlayer;

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

	UFUNCTION()
		virtual void OnRep_INSProjReplicatedMovement();

	/** delegate call back for collision detect */
	UFUNCTION()
		virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		virtual void CalAndSpawnPenetrateProjectile(const FHitResult& OriginHitResult, const FVector& OriginVelocity);
protected:
	// ~Begin AActor interface

	/** event begin play happens */
	virtual void BeginPlay() override;

	/** called every frame*/
	virtual void Tick(float DeltaSeconds)override;

	/** called after all components being initiated */
	virtual void PostInitializeComponents()override;

	virtual void PostNetReceiveVelocity(const FVector& NewVelocity)override;

	virtual void PostNetReceiveLocationAndRotation()override;

	/** Gather current INS Movement,override default FRepmentRep */
	virtual void GatherCurrentMovement()override;

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	// ~End AActor interface

	/** instantiate a client fake projectile for providing visual effect purpose */
	virtual void InitClientFakeProjectile();


public:

	/** return Hit Result */
	virtual FHitResult GetHitResult()const { return ImpactHit; }

	/** return base damage amount */
	inline virtual float GetBaseDamage()const { return DamageBase; }

	inline virtual uint8 GetCurrentPenetrateCount()const { return CurrentPenetrateCount; }

	virtual void SetCurrentPenetrateCount(uint8 AddInCount = 1){ CurrentPenetrateCount += AddInCount; }

	/** set owner weapon */
	virtual void SetOwnerWeapon(class AINSWeaponBase* NewWeaponOwner) { this->OwnerWeapon = NewWeaponOwner; }

	/** set instigator player that fires this projectile */
	virtual void SetInstigatedPlayer(class AController* InsigatedPlayer) { InstigatorPlayer = InsigatedPlayer; }

	/** set is fake projectile */
	virtual void SetIsFakeProjectile(bool bFake) { this->bClientFakeProjectile = bFake; }

	/** get is fake projectile */
	inline virtual bool GetIsFakeProjectile()const { return bClientFakeProjectile; }

	/** get owner weapon */
	virtual class AINSWeaponBase* GetOwnerWeapon()const { return OwnerWeapon; }

	/** get actual damage taken */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BFProjectile")
		virtual float GetDamageTaken();

	/** return collision comp */
	FORCEINLINE USphereComponent* GetCollsioncomp()const { return CollisionComp; }

	/** return mesh comp */
	FORCEINLINE UStaticMeshComponent* GetProjectileMesh()const { return ProjectileMesh; }

	/** return projectile movement comp */
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement()const { return ProjectileMoveComp; }
};
