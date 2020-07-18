// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "INSImpactEffect.generated.h"

class UParticleSystem;
class USoundCue;
class UMaterial;

USTRUCT(BlueprintType)
struct FImpactParticleData {

	GENERATED_USTRUCT_BODY()
		/** projectile Impact Particles*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* DefaultParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* DirtParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* ConcretParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* ThinMetalParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* ThickMetalParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* GrassParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* GlassParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* WaterParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* WoodParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* AlsphatParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* RuberParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* FleshParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* GravelParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* MudParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* CanParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* FabricParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* PlasticParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* CarpetParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* TreeLogParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* TreeLeavesParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactParticles")
		UParticleSystem* ArmorParticle;

};

USTRUCT(BlueprintType)
struct FImpactSoundData {

	GENERATED_USTRUCT_BODY()
		/** projectile impact SoundCue*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* DefaultSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* DirtSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* ConcretSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* ThinMetalSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* ThickMetalSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* GrassSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* GlassSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* WaterSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* WoodSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* AlsphatSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* RuberSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* FleshSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* GravelSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* MudSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* CanSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* FabricSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* PlasticSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* CarpetSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* TreeLogSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* TreeLeavesSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ImpactSound")
		USoundCue* ArmorSound;

};

USTRUCT(BlueprintType)
struct FImpactDecalData {
	GENERATED_USTRUCT_BODY()

	/** projectile impact Decal*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* DefaultDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* DirtDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* ConcreteDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* MetalDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* GlassDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* WoodDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* AlsphatDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* RubberDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Decals")
		UMaterial* FleshDecal;
};
UCLASS()
class INSURGENCY_API AINSImpactEffect : public AActor
{
	GENERATED_UCLASS_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ParticleData")
		FImpactParticleData ImpactParticles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ParticleData")
		FImpactSoundData ImpactSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ParticleData")
		FImpactDecalData ImpactDecals;

protected:
	virtual void PostInitializeComponents()override;

	virtual UParticleSystem* GetImpactParticlesBySurfaceType(EPhysicalSurface SurfaceType);

	virtual USoundCue* GetImpactSoundBySurfaceType(EPhysicalSurface SurfaceType);

	virtual UMaterial* GetDecalMaterialBySurfaceType(EPhysicalSurface surfaceType);

protected:
	FHitResult ImpactHit;

	class UPhysicalMaterial* PhysicMat;

public:
	virtual void SetImpactHit(const FHitResult& InImpactHit) { this->ImpactHit = InImpactHit; };

	virtual void SetPysicalMat(UPhysicalMaterial* NewPhysicMat);
};
