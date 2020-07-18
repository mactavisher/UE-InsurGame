// Fill out your copyright notice in the Description page of Project Settings.


#include "INSEffects/INSProjectileShell.h"
#include "INSEffects/INSImpactEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AINSProjectileShell::AINSProjectileShell(const FObjectInitializer& Objectinitializer):Super(Objectinitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ShellParticleComp"));
	RootComponent = ParticleComp;
	bCollided = false;

}

// Called when the game starts or when spawned
void AINSProjectileShell::BeginPlay()
{
	Super::BeginPlay();
	
}

void AINSProjectileShell::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ParticleCollideDelegate.BindUFunction(this, TEXT("OnShellCollide"));
	ParticleComp->OnParticleCollide.AddUnique(ParticleCollideDelegate);
	SetLifeSpan(10.f);
}

void AINSProjectileShell::OnShellCollide(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat)
{
	if (bCollided)
	{
		return;
		//make sure only on can entry for followed 
	}
	if (!ShellCollideEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to spawn projectile shell impact effects failed , no effect class Assigned!!"));
		return;
	}
	else
	{
		AINSImpactEffect* const ImpactActor = GetWorld()->SpawnActorDeferred<AINSImpactEffect>(ShellCollideEffectClass, GetActorTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		ImpactActor->SetPysicalMat(PhysMat);
		UGameplayStatics::FinishSpawningActor(ImpactActor, ImpactActor->GetActorTransform());
		bCollided = true;
		ParticleCollideDelegate.Unbind();
	}
}

