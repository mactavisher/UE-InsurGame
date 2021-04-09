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

DEFINE_LOG_CATEGORY(LogINSProjectile);
// Sets default values
AINSProjectile::AINSProjectile(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
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
	bIsGatheringMovement = false;
	MovementRepInterval = 0.1f;
	LastMovementRepTime = 0.f;
	bForceMovementReplication = true;
	bIsProcessingHit = false;
	CollisionComp->SetCollisionProfileName(FName(TEXT("Projectile")));
	/** turn this on because projectile's collision should be accurate to detect hit events and it's fast moving  */
	CollisionComp->SetAllUseCCD(true);
	CollisionComp->SetSphereRadius(5.f);
	bClientFakeProjectile = false;
	TraceScale = FVector::OneVector;
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
			AINSImpactEffect* EffctActor = GetWorld()->SpawnActorDeferred<AINSImpactEffect>(PointImapactEffectsClass,
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
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		if (bUsingDebugTrace)
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 3.0f);
		}
#endif
	}
	if (ClientFakeProjectile)
	{
		ClientFakeProjectile->Destroy();
	}
}

void AINSProjectile::OnRep_OwnerWeapon()
{
	if (OwnerWeapon)
	{
		InitClientFakeProjectile();
	}
}

void AINSProjectile::EnableFakeProjVisual()
{
	GetProjectileMesh()->SetHiddenInGame(false);
}

void AINSProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority&&!bIsProcessingHit)
	{
		TGuardValue<bool> HitGuard(bIsProcessingHit, true);
		GetProjectileMovementComp()->StopMovementImmediately();
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		UE_LOG(LogINSProjectile,
			Log,
			TEXT("Projectile hit something,this hit component name:%s,hit other component name:%s,other actor name :%s")
			, HitComponent == nullptr ? TEXT("NULL") : *HitComponent->GetName()
			, OtherComp == nullptr ? TEXT("NULL") : *OtherComp->GetName()
			, OtherActor == nullptr ? TEXT("NULL") : *OtherActor->GetName());
		if (OtherActor && OtherActor->GetClass()->IsChildOf(AINSCharacter::StaticClass()))
		{
			AINSCharacter* HitCharacter = CastChecked<AINSCharacter>(OtherActor);
			FPointDamageEvent PointDamageEvent;
			PointDamageEvent.HitInfo = Hit;
			PointDamageEvent.Damage = DamageBase;
			PointDamageEvent.ShotDirection = this->GetVelocity().GetSafeNormal();
			HitCharacter->TakeDamage(DamageBase, PointDamageEvent, InstigatorPlayer.Get(), this);
		}
		bForceMovementReplication = true;
		bIsGatheringMovement = false;
		GatherCurrentMovement();
		//force a transformation update to clients
		HitCounter++;
		UE_LOG(LogINSProjectile
			, Log
			, TEXT("Projectile%s Hit Happened,Will Force Send a Movement info to all Conected Clients")
			, *GetName());
		if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
		{
			OnRep_HitCounter();
		}
		//CalAndSpawnPenetrateProjectile(Hit, GetVelocity());
		SetLifeSpan(1.f);
		bIsProcessingHit = false;
	}
}

/*void AINSProjectile::CalAndSpawnPenetrateProjectile(const FHitResult& OriginHitResult, const FVector& OriginVelocity)
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
	GetWorld()->LineTraceSingleByChannel(
		PrececionHit,
		TraceStart,
		TraceEnd,
		ECollisionChannel::ECC_Camera,
		CollisionQueryParam, ECR_Block);
	if (PrececionHit.bBlockingHit)
	{
		const UPhysicalMaterial* const HitMat = PrececionHit.PhysMaterial.Get();
		EPhysicalSurface HitSurface = UPhysicalMaterial::DetermineSurfaceType(HitMat);
		float PenetrateVelocitySize = 0.f;
		switch (HitSurface)
		{
		case SurfaceType_Default:PenetrateVelocitySize = HitMat->Density * (FVector::Distance(OriginHitResult.Location, PrececionHit.Location) / 10) * 0.6f * OriginVelocity.Size();
		}
		if (PenetrateVelocitySize > MinPenetrateVelSize)
		{
			FTransform SpawnTransform(
				GetActorForwardVector().ToOrientationRotator(),
				PrececionHit.Location +
				GetActorForwardVector() * 25.f,
				FVector::OneVector);
			AINSProjectile* PenetratedProj = GetWorld()->SpawnActorDeferred<AINSProjectile>(
				this->GetClass(),
				SpawnTransform,
				this->GetOwnerWeapon()->GetOwnerCharacter()->GetController(),
				this->GetOwnerWeapon()->GetOwnerCharacter(),
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (PenetratedProj && GetCurrentPenetrateCount() < PenetrateCountThreshold)
			{
				PenetratedProj->GetProjectileMovementComp()->InitialSpeed = PenetrateVelocitySize;
				PenetratedProj->DamageBase = DamageBase * 0.7f;
				PenetratedProj->SetCurrentPenetrateCount(GetCurrentPenetrateCount() + 1);
				PenetratedProj->SetOwnerWeapon(this->GetOwnerWeapon());
				PenetratedProj->SetIsFakeProjectile(false);
				PenetratedProj->SetInstigatedPlayer(InstigatorPlayer.Get());
				UGameplayStatics::FinishSpawningActor(PenetratedProj, SpawnTransform);
			}
		}
	}
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	if (bUsingDebugTrace)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 10.f);
		DrawDebugSphere(GetWorld(), PrececionHit.Location + GetActorForwardVector() * 25.f, 5.f, 10, FColor::Red, false, 10.f);
	}
#endif
}*/

// Called when the game starts or when spawned
void AINSProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (!bClientFakeProjectile)
	{
		SetActorHiddenInGame(true);
	}
	if (GetClientFakeProjectile())
	{

		if (GetClientFakeProjectile()->CurrentPenetrateCount > 1)
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
	if (GetLocalRole() == ROLE_Authority)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, TEXT("OnProjectileHit"));
		CollisionComp->OnComponentHit.AddUnique(Delegate);
	}
	if (GetIsFakeProjectile())
	{
		TracerPaticleSizeTickFun.bCanEverTick;
		TracerPaticleSizeTickFun.SetTickFunctionEnable(true);
		TracerPaticleSizeTickFun.bTickEvenWhenPaused = true;
		TracerPaticleSizeTickFun.RegisterTickFunction(GetLevel());
		TracerPaticleSizeTickFun.Target = this;
		//GetProjectileMovementComp()->PrimaryComponentTick.AddPrerequisite(this);
	}
}

void AINSProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	Super::PostNetReceiveVelocity(NewVelocity);
	if (GetClientFakeProjectile())
	{
		GetProjectileMovementComp()->Velocity = NewVelocity;
		GetClientFakeProjectile()->GetProjectileMovementComp()->Velocity = NewVelocity;
	}
}

void AINSProjectile::PostNetReceiveLocationAndRotation()
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

void AINSProjectile::GatherCurrentMovement()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	const bool bRepDeltaTimeAllowed = GetWorld()->GetRealTimeSeconds() - LastMovementRepTime >= MovementRepInterval;
	if (bRepDeltaTimeAllowed || bForceMovementReplication)
	{
		if (bIsGatheringMovement)
		{
			return;
		}
		else
		{
			bIsGatheringMovement = true;
			Super::GatherCurrentMovement();
			FRepMovement OptRepMovement = GetReplicatedMovement();
			OptRepMovement.bRepPhysics = true;
			OptRepMovement.LocationQuantizationLevel = bForceMovementReplication ? EVectorQuantization::RoundWholeNumber : EVectorQuantization::RoundOneDecimal;
			OptRepMovement.RotationQuantizationLevel = ERotatorQuantization::ByteComponents;
			OptRepMovement.VelocityQuantizationLevel = bForceMovementReplication ? EVectorQuantization::RoundWholeNumber : EVectorQuantization::RoundOneDecimal;
			SetReplicatedMovement(OptRepMovement);
			if (bForceMovementReplication)
			{
				bForceMovementReplication = false;
			}
			bIsGatheringMovement = false;
			LastMovementRepTime = GetWorld()->GetRealTimeSeconds();
		}
	}
	//****************************************************************************************************/
	//      API Deprecation handling version
	//      compress and less byte will be sent
	// 	    PRAGMA_DISABLE_DEPRECATION_WARNINGS
	// 		ReplicatedMovement.bRepPhysics = true;
	// 		ReplicatedMovement.LocationQuantizationLevel = EVectorQuantization::RoundOneDecimal;
	// 		ReplicatedMovement.RotationQuantizationLevel = ERotatorQuantization::ByteComponents;
	// 		ReplicatedMovement.VelocityQuantizationLevel = EVectorQuantization::RoundOneDecimal;
	// 		PRAGMA_DISABLE_DEPRECATION_WARNINGS
	// 		LastMovementRepTime = GetWorld()->GetTimeSeconds();
	//***************************************************************************************************/
}

void AINSProjectile::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
}

void AINSProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSProjectile, HitCounter);
	DOREPLIFETIME(AINSProjectile, OwnerWeapon);
	DOREPLIFETIME(AINSProjectile, CurrentPenetrateCount);
}

void AINSProjectile::OnRep_ReplicatedMovement()
{
	if (GetClientFakeProjectile())
	{
		GetClientFakeProjectile()->SetReplicatedMovement(GetReplicatedMovement());
	}
	Super::OnRep_ReplicatedMovement();
}

void AINSProjectile::InitClientFakeProjectile()
{
	FVector SpawnLoc(ForceInit);
	FVector SpawDir(ForceInit);
	if (GetOwnerWeapon()->GetLocalRole()==ROLE_AutonomousProxy||GetLocalRole()==ROLE_Authority)
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
	ClientFakeProjectile = GetWorld()->SpawnActorDeferred<AINSProjectile>(
		this->GetClass(),
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (ClientFakeProjectile)
	{
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		GetClientFakeProjectile()->bUsingDebugTrace = false;
#endif
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

void AINSProjectile::SendInitialReplication()
{

}

void AINSProjectile::SetMuzzleSpeed(float NewSpeed)
{
	ProjectileMoveComp->InitialSpeed = NewSpeed;
}

void AINSProjectile::SetOwnerWeapon(class AINSWeaponBase* NewWeaponOwner)
{
	this->OwnerWeapon = NewWeaponOwner;
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
		if (GetActorLocation().Z >= 150000.f || GetActorLocation().Z <= -50000.f)
		{
			Destroy(false, true);
		}
	}
}

void AINSProjectile::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	if (&ThisTickFunction == &TracerPaticleSizeTickFun)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, TEXT("TickActor calling Tracer scale"));
		FVector CurrentTracerScale = TracerParticle->GetComponentTransform().GetScale3D();
		float ScaleX = CurrentTracerScale.X;
		float ScaleY = CurrentTracerScale.Y;
		float ScaleZ = CurrentTracerScale.Z;
		FVector NewScale(FMath::Clamp<float>(ScaleX += DeltaTime*100, ScaleX, 10.f)
			, ScaleY
			, FMath::Clamp<float>(ScaleZ += DeltaTime, ScaleZ, 2.f));
		TracerParticle->SetWorldScale3D(NewScale);
		if (ScaleX >= 20.f)
		{
			TracerPaticleSizeTickFun.SetTickFunctionEnable(false);
			TracerPaticleSizeTickFun.UnRegisterTickFunction();
		}
	}
}
