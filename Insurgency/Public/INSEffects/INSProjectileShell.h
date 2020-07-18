// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "INSProjectileShell.generated.h"


class UParticleSystemComponent;
class AINSImpactEffect;

UCLASS()
class INSURGENCY_API AINSProjectileShell : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	// Sets default values for this actor's properties
	AINSProjectileShell();
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ShellParticle")
		UParticleSystemComponent* ParticleComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AmmoShell|ShellParticle")
		UParticleSystem* ParticleTemplate;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Effects")
	   TSubclassOf<AINSImpactEffect> ShellCollideEffectClass;


	UPROPERTY()
		uint8 bCollided : 1;

	UPROPERTY()
		int32 CurrentCollideCount;

	FScriptDelegate ParticleCollideDelegate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents()override;
	UFUNCTION()
		virtual void OnShellCollide(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat);
};
