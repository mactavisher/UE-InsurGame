// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicalMaterials\PhysicalMaterial.h"
#include "INSProjectileShell.generated.h"


class UParticleSystemComponent;
class AINSImpactEffect;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(INSProjectileShell, Log, All);

UCLASS(notplaceable, Blueprintable)
class INSURGENCY_API AINSProjectileShell : public AActor
{
	GENERATED_UCLASS_BODY()
protected:
	/** shell particle comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ShellParticle")
	UParticleSystemComponent* ParticleComp;

	/** shell particle template */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AmmoShell|ShellParticle")
	UParticleSystem* ParticleTemplate;

	/** shell particle impact spawn effect class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<AINSImpactEffect> ShellCollideEffectClass;

	/** indicate if this particle comp collided */
	UPROPERTY()
	uint8 bCollided : 1;

	UPROPERTY()
	int32 CurrentCollideCount;

	FScriptDelegate ParticleCollideDelegate;

protected:
	//~ begin AActor interface
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	//~ end AActor interface

	/**
	 * call back function call back when this particle comp collide
	 * @param EventName EventName
	 * @param EmitterTime EmitterTime
	 * @param ParticleTime ParticleTime
	 * @param Location Collide Location
	 * @param Direction Collide Direction
	 * @param Normal collide normal
	 * @param BoneName collide bone name if any
	 * @param PhysMat Collide surface material type
	 */
	UFUNCTION()
	virtual void OnCollide(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat);
};
