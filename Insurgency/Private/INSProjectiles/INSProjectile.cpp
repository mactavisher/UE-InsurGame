// Fill out your copyright notice in the Description page of Project Settings.


#include "INSProjectiles/INSProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "INSEffects/INSImpactEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "Insurgency/Insurgency.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

DEFINE_LOG_CATEGORY(LogINSProjectile);
// Sets default values
AINSProjectile::AINSProjectile(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.SetTickFunctionEnable(false);
	SetReplicates(true);
	HitCounter = 0;
	bIsExplosive = false;
	DamageBase = 20.f;
	CollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	CurrentPenetrateCount = 0;
	PenetrateCountThreshold = 3;
	ProjectileMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComp"));
	ProjectileMoveComp = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileMovementComp"));
	TracerParticle = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("TraceParticle"));
	ProjectileMoveComp->UpdatedComponent = CollisionComp;
	ProjectileMesh->SetupAttachment(CollisionComp);
	TracerParticle->SetupAttachment(CollisionComp);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TracerParticle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InitialLifeSpan = 5.f;
	SetReplicatingMovement(false);
	bRepINSMovement = false;
	CollisionComp->SetCollisionProfileName(FName(TEXT("Projectile")));
	/** turn this on because projectile's collision should be accurate to detect hit events and it's fast moving  */
	CollisionComp->SetAllUseCCD(true);
	CollisionComp->SetSphereRadius(5.f);
	bClientFakeProjectile = false;
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	bUsingDebugTrace = true;
#endif
}

void AINSProjectile::OnRep_Explode()
{

}

void AINSProjectile::OnRep_HitCounter()
{
	if (HitCounter > 0)
	{
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		if (ClientFakeProjectile)
		{
			QueryParams.AddIgnoredActor(ClientFakeProjectile);
		}
		QueryParams.AddIgnoredActor(GetOwnerWeapon());
		QueryParams.AddIgnoredActor(GetOwnerWeapon()->GetOwnerCharacter());
		QueryParams.bReturnPhysicalMaterial = true;
		const FVector ProjectileLoc = bClientFakeProjectile?INSProjReplicatedMovement.Location:GetActorLocation();
		const FVector ProjectileDir = GetActorForwardVector();
		const FVector TraceStart = ProjectileLoc + (-ProjectileDir * 200.f);
		const float TraceRange = 1000.f;
		const FVector TraceEnd = TraceStart + ProjectileDir * TraceRange;
		GetWorld()->LineTraceSingleByChannel(ImpactHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);
		const AActor* HitActor = ImpactHit.Actor.Get();
		UClass* HitActorClass = HitActor == nullptr ? nullptr : HitActor->GetClass();
		if (ImpactHit.bBlockingHit 
			&& HitActor 
			&& HitActorClass 
			&& !HitActorClass->IsChildOf(AINSCharacter::StaticClass()) //not character ,because it already handles visual effects if self on Rep_LastHitInfo
			&& !HitActorClass->IsChildOf(AINSWeaponBase::StaticClass()))
		{
			FTransform EffctActorTransform(ImpactHit.ImpactNormal.ToOrientationRotator(), ImpactHit.ImpactPoint, FVector(1.f, 1.f, 1.f));
			AINSImpactEffect* const EffctActor = GetWorld()->SpawnActorDeferred<AINSImpactEffect>(PointImapactEffectsClass,
				EffctActorTransform,
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (EffctActor)
			{
				EffctActor->SetImpactHit(ImpactHit);
				UGameplayStatics::FinishSpawningActor(EffctActor, EffctActorTransform);
			}
		}
	}
	if (ClientFakeProjectile)
	{
		ClientFakeProjectile->Destroy(true);
	}
}

void AINSProjectile::OnRep_OwnerWeapon()
{
	if (OwnerWeapon)
	{
		if (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			return;
		}
		InitClientFakeProjectile();
	}
}

void AINSProjectile::EnableFakeProjVisual()
{
	GetProjectileMesh()->SetHiddenInGame(false);
}

void AINSProjectile::OnRep_INSProjReplicatedMovement()
{
	FRepMovement NewReplicatedMovement;
	NewReplicatedMovement.Location = INSProjReplicatedMovement.Location;
	NewReplicatedMovement.Rotation = INSProjReplicatedMovement.Rotation;
	NewReplicatedMovement.LinearVelocity = INSProjReplicatedMovement.LinearVelocity;
	NewReplicatedMovement.AngularVelocity = FVector(0.f);
	NewReplicatedMovement.bSimulatedPhysicSleep = false;
	NewReplicatedMovement.bRepPhysics = false;
	NewReplicatedMovement.VelocityQuantizationLevel = EVectorQuantization::RoundOneDecimal;
	NewReplicatedMovement.RotationQuantizationLevel = ERotatorQuantization::ByteComponents;
	SetReplicatedMovement(NewReplicatedMovement);
	OnRep_ReplicatedMovement();
}

void AINSProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		HitCounter = HitCounter + 1;
		UE_LOG(LogINSProjectile, 
			Log, 
			TEXT("Projectile hit something,this hit component name:%s,hit other component name:%s,other actor name :%s") 
			,*HitComponent->GetName() 
			,*OtherComp->GetName()
			,*OtherActor->GetName());
		if (OtherActor&&OtherActor->GetClass()->IsChildOf(AINSCharacter::StaticClass()))
		{
			AINSCharacter* HitCharacter = CastChecked<AINSCharacter>(OtherActor);
			FHitResult PoitHit;
			FPointDamageEvent PointDamageEvent;
			PoitHit.Actor = HitCharacter;
			PoitHit.Location = Hit.Location;
			PoitHit.ImpactPoint = Hit.ImpactPoint;
			PoitHit.ImpactNormal = Hit.ImpactNormal;
			PointDamageEvent.HitInfo = PoitHit;
			PointDamageEvent.ShotDirection = this->GetVelocity().GetSafeNormal();
			HitCharacter->ReceiveHit(InstigatorPlayer.Get(), this, PointDamageEvent, Hit, DamageBase);
		}
		if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
		{
			OnRep_HitCounter();
		}
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//CalAndSpawnPenetrateProjectile(Hit, GetVelocity());
		SetLifeSpan(0.02f);
	}
}

void AINSProjectile::CalAndSpawnPenetrateProjectile(const FHitResult& OriginHitResult, const FVector& OriginVelocity)
{
	const float PenetrateMinDistance = 2.f;
	const float PenetrateMaxThresholdRange = 30.f;
	const float MinPenetrateVelSize = 5000.f;
	FHitResult PrececionHit;
	const FVector TraceDir = GetActorForwardVector();
	const FVector TraceStart = OriginHitResult.Location + TraceDir * 2.f;
	const FVector TraceEnd = TraceStart + TraceDir * PenetrateMaxThresholdRange;
	FCollisionQueryParams CollisionQueryParam;
	CollisionQueryParam.AddIgnoredActor(this);
	CollisionQueryParam.bReturnPhysicalMaterial = true;
	GetWorld()->LineTraceSingleByChannel(PrececionHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Camera, CollisionQueryParam, ECR_Block);
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 10.f);
	if (PrececionHit.bBlockingHit)
	{
		const UPhysicalMaterial* const HitMat = PrececionHit.PhysMaterial.Get();
		EPhysicalSurface HitSurface = UPhysicalMaterial::DetermineSurfaceType(HitMat);
		float PenetrateVelocitySize = 0.f;
		switch (HitSurface)
		{
		case SurfaceType_Default:PenetrateVelocitySize = HitMat->Density*(FVector::Distance(OriginHitResult.Location, PrececionHit.Location) / 10)*0.6f*OriginVelocity.Size();
		}
		if (PenetrateVelocitySize > MinPenetrateVelSize)
		{
			FTransform SpawnTransform(GetActorForwardVector().ToOrientationRotator(), PrececionHit.Location + GetActorForwardVector()*25.f, FVector(1.f, 1.f, 1.f));
			DrawDebugSphere(GetWorld(), PrececionHit.Location + GetActorForwardVector()*25.f, 5.f, 10, FColor::Red, false, 10.f);
			AINSProjectile* const PenetratedProj = GetWorld()->SpawnActorDeferred<AINSProjectile>(this->GetClass(), SpawnTransform,
				this->GetOwnerWeapon()->GetOwnerCharacter()->GetController(),
				this->GetOwnerWeapon()->GetOwnerCharacter(),
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (PenetratedProj&&GetCurrentPenetrateCount()<PenetrateCountThreshold)
			{
				PenetratedProj->GetProjectileMovement()->InitialSpeed = PenetrateVelocitySize;
				PenetratedProj->DamageBase = DamageBase * 0.7f;
				PenetratedProj->SetCurrentPenetrateCount(GetCurrentPenetrateCount() + 1);
				PenetratedProj->SetOwnerWeapon(this->GetOwnerWeapon());
				PenetratedProj->SetIsFakeProjectile(false);
				PenetratedProj->SetInstigatedPlayer(InstigatorPlayer.Get());
				UGameplayStatics::FinishSpawningActor(PenetratedProj, SpawnTransform);
			}
		}
	}
}

// Called when the game starts or when spawned
void AINSProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority && !bClientFakeProjectile)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, TEXT("OnProjectileHit"));
		CollisionComp->OnComponentHit.AddUnique(Delegate);
	}
	if (bClientFakeProjectile)
	{
		
		if (CurrentPenetrateCount >1)
		{
			GetProjectileMesh()->SetHiddenInGame(true);
			GetWorldTimerManager().SetTimer(FakeProjVisibilityTimer, this, &AINSProjectile::EnableFakeProjVisual, 1.f, false, 0.5f);
		}
		else
		{
			//hide this fake projectile for one frame-time
			GetProjectileMesh()->SetHiddenInGame(false);
		}
	}
}

void AINSProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AINSProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	Super::PostNetReceiveVelocity(NewVelocity);
	if (ClientFakeProjectile)
	{
		ClientFakeProjectile->GetProjectileMovement()->Velocity = NewVelocity;
	}
}

void AINSProjectile::PostNetReceiveLocationAndRotation()
{
	Super::PostNetReceiveLocationAndRotation();
	if (ClientFakeProjectile)
	{
		ClientFakeProjectile->SetActorLocationAndRotation(INSProjReplicatedMovement.Location, INSProjReplicatedMovement.Rotation);
	}
}

void AINSProjectile::GatherCurrentMovement()
{
	if (RootComponent != nullptr)
	{
		// If we are attached, don't replicate absolute position
		if (RootComponent->GetAttachParent() != nullptr)
		{
			Super::GatherCurrentMovement();
		}
		else
		{
			INSProjReplicatedMovement.Location = RootComponent->GetComponentLocation();
			INSProjReplicatedMovement.Rotation = RootComponent->GetComponentRotation();
			INSProjReplicatedMovement.LinearVelocity = GetVelocity();
		}
	}
}

void AINSProjectile::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	if ((bRepINSMovement) && (GetLocalRole() == ROLE_Authority))
	{
		GatherCurrentMovement();
	}
}

void AINSProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSProjectile, HitCounter);
	DOREPLIFETIME(AINSProjectile, OwnerWeapon);
	DOREPLIFETIME(AINSProjectile, INSProjReplicatedMovement);
	DOREPLIFETIME(AINSProjectile, CurrentPenetrateCount);
}

void AINSProjectile::InitClientFakeProjectile()
{
	this->SetActorHiddenInGame(true);
	FVector spawnLoc(ForceInit);
	FVector spawDir = GetActorForwardVector();
	if (GetOwnerWeapon()->GetIsOwnerLocal())
	{
		spawnLoc = GetOwnerWeapon()->WeaponMesh1PComp->GetMuzzleLocation() + 100.f*spawDir;
	}
	else
	{
		spawnLoc = GetOwnerWeapon()->WeaponMesh3PComp->GetMuzzleLocation() + 100.f*spawDir;
	}
	FTransform SpawnTransform(spawDir.ToOrientationRotator(), spawnLoc, FVector(1.f, 1.f, 1.f));
	ClientFakeProjectile = GetWorld()->SpawnActorDeferred<AINSProjectile>(this->GetClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (ClientFakeProjectile)
	{
		ClientFakeProjectile->SetIsFakeProjectile(true);
		ClientFakeProjectile->GetCollsioncomp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ClientFakeProjectile->GetProjectileMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ClientFakeProjectile->GetCollsioncomp()->SetAllUseCCD(false);
		ClientFakeProjectile->SetCurrentPenetrateCount(GetCurrentPenetrateCount() + 1);
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		ClientFakeProjectile->bUsingDebugTrace = false;
#endif
		UGameplayStatics::FinishSpawningActor(ClientFakeProjectile, SpawnTransform);
	}
}

float AINSProjectile::GetDamageTaken()
{
	return 0.f;
}


// Called every frame
void AINSProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bClientFakeProjectile)
	{
		//invalid shot,just destroy it
		if (GetActorLocation().Z >= 150000.f|| GetActorLocation().Z <= 50000.f)
		{
			Destroy(true);
		}
	}
}