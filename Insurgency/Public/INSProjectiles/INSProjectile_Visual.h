// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSProjectileBase.h"
#include "INSProjectile_Visual.generated.h"

class AINSImpactEffect;
class USphereComponent;
class UINSProjectileMovementComponent;
class AINSProjectile_Server;
class AINSWeaponBase;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogVisualProjectile, Log, All);

UCLASS()
class INSURGENCY_API AINSProjectile_Visual : public AINSProjectileBase
{
	GENERATED_UCLASS_BODY()
protected:
	/** projectile trace effect  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, category = "Effects|Trace", meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* TracerParticleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NetWorkOwner")
	AINSProjectile_Server* ServerProjectile;

	/** radius impact effect class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactEffects")
	TSubclassOf<AINSImpactEffect> ImpactEffectsClass;

	FActorTickFunction ProjectileHiddenTickFun;

	UPROPERTY()
	FTransform ServerProjectileTrans;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual void PostInitializeComponents() override;
	virtual void SpawnImpactHit(const FHitResult& Hit);
	virtual void RegisterProjectileHiddenTick(const bool bDoRegister);
	/** delegate call back for collision detect */
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
public:
	virtual void OnServerProjectileHit(const FHitResult& HitResult);
	virtual void SetServerProjectile(AINSProjectile_Server* NewServerProjectile);
	virtual void SetIsScanTraceProjectile(const bool bScanTrace) override;
	virtual void ReceiveServerProjectileHit();
	virtual void SetScanTraceTime(const float NewTime) override;
	virtual void ReceiveServerProjectileTransform(FTransform& InServerProjectileTrans);
};
