// Fill out your copyright notice in the Description page of Project Settings.


#include "INSProjectiles/INSProjectile_Stable.h"
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

DEFINE_LOG_CATEGORY(LogINSProjectile_Stable);

// Sets default values
AINSProjectile_Stable::AINSProjectile_Stable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
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
	SetReplicatingMovement(true);
	//bIsGatheringMovement = false;
	//MovementRepInterval = 0.1f;
	//LastMovementRepTime = 0.f;
	//bForceMovementReplication = true;
	CollisionComp->SetCollisionProfileName(FName(TEXT("Projectile")));
	/** turn this on because projectile's collision should be accurate to detect hit events and it's fast moving  */
	CollisionComp->SetAllUseCCD(true);
	CollisionComp->SetSphereRadius(5.f);
	bClientFakeProjectile = false;
}

void AINSProjectile_Stable::OnRep_Explode()
{

}

void AINSProjectile_Stable::OnRep_HitCounter()
{
	if (HitCounter > 0)
	{
		FVector ProjectileLoc(ForceInit);
		FVector ProjectileDir(ForceInit);
		if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
		{
			ProjectileLoc = GetActorLocation();
			ProjectileDir = GetActorForwardVector();
		}
		else if (GetNetMode() == ENetMode::NM_Client)
		{
			if (GetClientFakeProjectile())
			{
				GetClientFakeProjectile()->GetProjectileMovementComp()->StopMovementImmediately();
				ProjectileDir = GetClientFakeProjectile()->GetReplicatedMovement().Rotation.Vector();
				ProjectileLoc = GetClientFakeProjectile()->GetReplicatedMovement().Location - ProjectileDir * 200.f;
			}
		}
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		if (ClientFakeProjectile)
		{
			QueryParams.AddIgnoredActor(ClientFakeProjectile);
		}
		QueryParams.AddIgnoredActor(GetOwnerWeapon());
		QueryParams.AddIgnoredActor(GetOwnerWeapon()->GetOwnerCharacter());
		QueryParams.bReturnPhysicalMaterial = true;
		const FVector TraceStart = ProjectileLoc;
		const float TraceRange = 1000.f;
		const FVector TraceEnd = TraceStart + ProjectileDir * TraceRange;
		GetWorld()->LineTraceSingleByChannel(ImpactHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Camera, QueryParams);
		const AActor* HitActor = ImpactHit.Actor.Get();
		UClass* HitActorClass = HitActor == nullptr ? nullptr : HitActor->GetClass();
		if (ImpactHit.bBlockingHit
			&& HitActor
			&& HitActorClass
			//not character ,because it already handles visual effects if self on Rep_LastHitInfo
			&& !HitActorClass->IsChildOf(AINSCharacter::StaticClass())
			&& !HitActorClass->IsChildOf(AINSWeaponBase::StaticClass()))
		{
			FTransform EffctActorTransform(ImpactHit.ImpactNormal.ToOrientationRotator(), ImpactHit.ImpactPoint, FVector::OneVector);
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
		ClientFakeProjectile->Destroy();
	}
}

void AINSProjectile_Stable::OnRep_OwnerWeapon()
{
	if (OwnerWeapon)
	{
		UE_LOG(LogINSProjectile_Stable,
			Log,
			TEXT("Projectile:%s Weapon owner Replicated,Init a client fake Projectile Now"));
		if (GetNetMode() == ENetMode::NM_ListenServer)
		{
			return;
		}
		InitClientFakeProjectile();
	}
}

void AINSProjectile_Stable::EnableFakeProjVisual()
{
	
}

void AINSProjectile_Stable::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ProjectHitDelegate.Unbind();
		UE_LOG(LogINSProjectile_Stable,
			Log,
			TEXT("Projectile hit something,this hit component name:%s,hit other component name:%s,other actor name :%s")
			, HitComponent == nullptr ? TEXT("NULL") : *HitComponent->GetName()
			, OtherComp == nullptr ? TEXT("NULL") : *OtherComp->GetName()
			, OtherActor == nullptr ? TEXT("NULL") : *OtherActor->GetName());
		if (OtherActor && OtherActor->GetClass()->IsChildOf(AINSCharacter::StaticClass()))
		{
			AINSCharacter* HitCharacter = CastChecked<AINSCharacter>(OtherActor);
			FHitResult PoitHit;
			FPointDamageEvent PointDamageEvent;
			PoitHit.Actor = HitCharacter;
			PoitHit.Location = Hit.Location;
			PoitHit.ImpactPoint = Hit.ImpactPoint;
			PoitHit.ImpactNormal = Hit.ImpactNormal;
			PointDamageEvent.HitInfo = PoitHit;
			PointDamageEvent.Damage = DamageBase;
			PointDamageEvent.ShotDirection = this->GetVelocity().GetSafeNormal();
			HitCharacter->TakeDamage(DamageBase, PointDamageEvent, InstigatorPlayer.Get(), this);
		}
		//force a transformation update to clients
		HitCounter++;
		UE_LOG(LogINSProjectile_Stable
			, Log
			, TEXT("Projectile%s Hit Happened,Will Force Send a Movement info to all Conected Clients")
			, *GetName());
		if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
		{
			OnRep_HitCounter();
		}
		//CalAndSpawnPenetrateProjectile(Hit, GetVelocity());
		SetLifeSpan(1.f);
		GetProjectileMovementComp()->StopMovementImmediately();
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}

void AINSProjectile_Stable::CalAndSpawnPenetrateProjectile(const FHitResult& OriginHitResult, const FVector& OriginVelocity)
{

}

void AINSProjectile_Stable::BeginPlay()
{
	Super::BeginPlay();
	
}

void AINSProjectile_Stable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Super::Tick(DeltaTime);
	if (!bClientFakeProjectile)
	{
		//invalid shot,just destroy it
		if (GetActorLocation().Z >= 150000.f || GetActorLocation().Z <= -50000.f)
		{
			Destroy(false, true);
		}
	}

}

void AINSProjectile_Stable::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//SetActorHiddenInGame(true);
	if (GetLocalRole() == ROLE_Authority)
	{
		ProjectHitDelegate.BindUFunction(this, TEXT("OnProjectileHit"));
		CollisionComp->OnComponentHit.AddUnique(ProjectHitDelegate);
	}
}

void AINSProjectile_Stable::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	Super::PostNetReceiveVelocity(NewVelocity);
	if (GetClientFakeProjectile())
	{
		GetProjectileMovementComp()->Velocity = NewVelocity;
		GetClientFakeProjectile()->GetProjectileMovementComp()->Velocity = NewVelocity;
	}
}

void AINSProjectile_Stable::PostNetReceiveLocationAndRotation()
{
	Super::PostNetReceiveLocationAndRotation();
	if (GetClientFakeProjectile())
	{
		SetActorLocationAndRotation(
			GetReplicatedMovement().Location,
			GetReplicatedMovement().Rotation);
		GetClientFakeProjectile()->SetActorLocationAndRotation(
			GetReplicatedMovement().Location,
			GetReplicatedMovement().Rotation);
	}
}

void AINSProjectile_Stable::GatherCurrentMovement()
{
	Super::GatherCurrentMovement();
}

void AINSProjectile_Stable::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
}

void AINSProjectile_Stable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSProjectile_Stable, HitCounter);
	DOREPLIFETIME(AINSProjectile_Stable, OwnerWeapon);
	DOREPLIFETIME(AINSProjectile_Stable, CurrentPenetrateCount);
}

void AINSProjectile_Stable::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
}

void AINSProjectile_Stable::InitClientFakeProjectile()
{
	FVector SpawnLoc(ForceInit);
	FVector SpawDir(ForceInit);
	if (GetOwnerWeapon()->GetLocalRole() == ROLE_AutonomousProxy || GetLocalRole() == ROLE_Authority)
	{
		SpawDir = GetOwnerWeapon()->WeaponMesh1PComp->GetMuzzleForwardVector();
		SpawnLoc = GetOwnerWeapon()->WeaponMesh1PComp->GetMuzzleLocation() + 10.f * SpawDir;
	}
	else
	{
		SpawDir = GetOwnerWeapon()->WeaponMesh3PComp->GetMuzzleForwardVector();
		SpawnLoc = GetOwnerWeapon()->WeaponMesh3PComp->GetMuzzleLocation() + 10.f * SpawDir;
	}
	const FTransform SpawnTransform(SpawDir.ToOrientationRotator(), SpawnLoc, FVector::OneVector);
	ClientFakeProjectile = GetWorld()->SpawnActorDeferred<AINSProjectile_Stable>(
		this->GetClass(),
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (ClientFakeProjectile)
	{
		GetClientFakeProjectile()->SetIsFakeProjectile(true);
		GetClientFakeProjectile()->NetAuthrotyProjectile = this;
		GetClientFakeProjectile()->SetOwnerWeapon(GetOwnerWeapon());
		GetClientFakeProjectile()->SetCurrentPenetrateCount(GetCurrentPenetrateCount() + 1);
		GetClientFakeProjectile()->GetCollsioncomp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetClientFakeProjectile()->GetCollsioncomp()->SetUseCCD(false);
		GetClientFakeProjectile()->GetCollsioncomp()->SetAllUseCCD(false);
		GetClientFakeProjectile()->SetActorLocation(GetActorLocation());
		GetClientFakeProjectile()->GetProjectileMovementComp()->InitialSpeed = GetClientFakeProjectile()->GetOwnerWeapon()->GetMuzzleSpeedValue();
		UGameplayStatics::FinishSpawningActor(ClientFakeProjectile, SpawnTransform);
	}
}

void AINSProjectile_Stable::SendInitialReplication()
{

}

void AINSProjectile_Stable::SetMuzzleSpeed(float NewSpeed)
{

}

float AINSProjectile_Stable::GetDamageTaken()
{
	return 0.f;
}
