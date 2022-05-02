// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "INSProjectileBase.generated.h"

class UINSProjectileMovementComponent;
class USphereComponent;
class AINSWeaponBase;

UCLASS()
class INSURGENCY_API AINSProjectileBase : public AActor
{
	GENERATED_UCLASS_BODY()
protected:
	/** collision comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CollisionComp", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionComp;

	/** ProjectileMovementComponent,this will drive the movement of this projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ProjecileMovementComp", meta = (AllowPrivateAccess = "true"))
	UINSProjectileMovementComponent* ProjectileMovementComponent;

	/** weapon that owns(fires)this projectile,and will get replicated*/
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_OwnerWeapon)
	AINSWeaponBase* OwnerWeapon;

	/** whether bScanTraceProjectile has been set and replicated */
	UPROPERTY()
	uint8 bScanTraceConditionSet:1;

	/** is this projectile hit by way of scan trace, if true ,we just need to tell client where it flies and no need movement replication or scale the the movement rep interval */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing= OnRep_ScanTraceCondition, Category = "Replication")
	uint8 bScanTraceProjectile : 1;

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_ScanTraceTime)
	float ScanTraceMoveTime;

	FScriptDelegate HitDelegate;

protected:
	//~Begin AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	//~End AActor interface

	//~Begin Rep call backs
	UFUNCTION()
	virtual void OnRep_ScanTraceCondition();
	UFUNCTION()
	virtual void OnRep_OwnerWeapon();
	UFUNCTION()
	virtual void OnRep_ScanTraceTime();
	//~Begin Rep call backs

	virtual void BindingHitDelegate(bool bDoBinding);

	/** delegate call back for collision detect */
	UFUNCTION()
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                             FVector NormalImpulse, const FHitResult& Hit);

public:
	virtual void SetIsScanTraceProjectile(const bool bScanTrace);
	virtual void SetOwnerWeapon(AINSWeaponBase* NewOwnerWeapon);
	virtual void SetMuzzleSpeed(float MuzzleSpeed);
	virtual void SetMoveIgnoredActor(TArray<AActor*> ActorsToIgnore);
	virtual bool GetScanTraceCondition() const { return bScanTraceConditionSet; }
	virtual void SetProjectileTick(const bool bEnableTick);
	virtual void SetScanTraceTime(const float NewTime);
	FORCEINLINE virtual UINSProjectileMovementComponent* GetProjectileMovementComponent() const
	{
		return ProjectileMovementComponent;
	}
};
