// Fill out your copyright notice in the Description page of Project Settings.


#include "INSEffects/INSProjectileShell.h"
#include "INSEffects/INSImpactEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

DEFINE_LOG_CATEGORY(INSProjectileShell);

// Sets default values
AINSProjectileShell::AINSProjectileShell(const FObjectInitializer& Objectinitializer) :Super(Objectinitializer)
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
	/*if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		ParticleCollideDelegate.BindUFunction(this, TEXT("OnShellCollide"));
		ParticleComp->OnParticleCollide.AddUnique(ParticleCollideDelegate);
	}
	SetLifeSpan(10.f);*/
}

/*void AINSProjectileShell::OnCollide(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat)
{
	/*if (bCollided)
	{
		UE_LOG(INSProjectileShell, Warning, TEXT("this projectile shell has finished it's collide events,abort!"));
		return;
		//make sure only on can entry for followed 
	}
	if (!ShellCollideEffectClass)
	{
		UE_LOG(INSProjectileShell, Warning, TEXT("Trying to spawn projectile shell impact effects failed , no effect class Assigned!!"));
		return;
	}
	else
	{
		// init the shell impact effect spawn transform
		const FTransform ImpactSpawnTrans(Velocity.ToOrientationQuat(), Location, FVector::OneVector);
		AINSImpactEffect* const ImpactActor = GetWorld()->SpawnActorDeferred<AINSImpactEffect>(ShellCollideEffectClass
			, ImpactSpawnTrans
			, nullptr
			, nullptr
			, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (ImpactActor)
		{
			FHitResult ImpactHit(ForceInit);
			ImpactHit.BoneName = BoneName;
			ImpactHit.Location = Location;
			ImpactHit.ImpactPoint = Location;
			ImpactHit.ImpactNormal = Normal;
			ImpactHit.PhysMaterial = PhysMat;
			ImpactActor->SetImpactHit(ImpactHit);
			UGameplayStatics::FinishSpawningActor(ImpactActor, ImpactSpawnTrans);
			bCollided = true;
			UE_LOG(INSProjectileShell
				, Warning
				, TEXT("projectile shell %s spawns it impacte effect at location %s,Spawned impact effct instance name %s")
				, *GetName()
				, *ImpactSpawnTrans.GetLocation().ToString()
				, *ImpactActor->GetName());
			ParticleCollideDelegate.Unbind();
		}
		else if (!ImpactActor)
		{
			UE_LOG(INSProjectileShell
				, Warning
				, TEXT("projectile shell %s spawns it impacte effect failed at transform %s")
				, *GetName()
				, *ImpactSpawnTrans.ToString());
		}
	}
}*/

