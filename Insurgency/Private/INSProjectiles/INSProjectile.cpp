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
#include "INSComponents/INSProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "Insurgency/Insurgency.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/ActorChannel.h"

DEFINE_LOG_CATEGORY(LogINSProjectile);

AINSProjectile::AINSProjectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	bReplicates = true;
	HitCounter = 0;
	bIsExplosive = false;
	DamageBase = 20.f;
	CollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	CurrentPenetrateCount = 0;
	PenetrateCountThreshold = 3;
	ProjectileMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComp"));
	ProjectileMoveComp = ObjectInitializer.CreateDefaultSubobject<UINSProjectileMovementComponent>(this, TEXT("ProjectileMovementComp"));
	TracerParticle = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("TraceParticle"));
	ProjectileMoveComp->UpdatedComponent = CollisionComp;
	ProjectileMesh->SetupAttachment(CollisionComp);
	TracerParticle->SetupAttachment(CollisionComp);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TracerParticle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TracerParticle->SetWorldScale3D(FVector(0.f, 0.f, 0.f));
	InitialLifeSpan = 10.f;
	SetReplicatingMovement(true);
	bIsGatheringMovement = false;
	MovementRepInterval = 0.5f;
	LastMovementRepTime = 0.f;
	bForceMovementReplication = true;
	bNeedPositionSync = false;
	bInPositionSync = false;
	bIsProcessingHit = false;
	bScanTraceProjectile = false;
	bDelayedHit = false;
	CollisionComp->SetCollisionProfileName(FName(TEXT("Projectile")));
	/** turn this on because projectile's collision should be accurate to detect hit events and it's fast moving  */
	CollisionComp->SetAllUseCCD(true);
	CollisionComp->SetSphereRadius(1.f);
	bVisualProjectile = true;
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	bUsingDebugTrace = true;
#endif
	InitRepTickFunc.bCanEverTick = true;
	InitRepTickFunc.bTickEvenWhenPaused = true;
	InitRepTickFunc.SetTickFunctionEnable(true);
	ProjectileMoveComp->PrimaryComponentTick.AddPrerequisite(this, InitRepTickFunc);
	PrimaryActorTick.AddPrerequisite(ProjectileMoveComp, ProjectileMoveComp->PrimaryComponentTick);
	MovementQuantizeLevel = EVectorQuantization::RoundWholeNumber;
}

void AINSProjectile::OnRep_Explode()
{
}

void AINSProjectile::OnRep_HitCounter()
{
	if (HitCounter > 0)
	{
		CheckImpactHit();
	}
}

void AINSProjectile::OnRep_OwnerWeapon()
{
	if (OwnerWeapon)
	{
		InitClientFakeProjectile(this);
	}
}

void AINSProjectile::EnableFakeProjVisual()
{
	GetProjectileMesh()->SetHiddenInGame(false);
}

void AINSProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority && !bIsProcessingHit)
	{
		TGuardValue<bool> HitGuard(bIsProcessingHit, true);
		GetProjectileMovementComp()->StopMovementImmediately();
		GetProjectileMovementComp()->SetComponentTickEnabled(false);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		bForceMovementReplication = true;
		HitCounter++;
		//force a transformation update to clients
		GatherCurrentMovement();
		ProjectileLiftTimeData.ImpactSpeed = GetVelocity().Size();
		ProjectileLiftTimeData.ImpactActor = Hit.GetActor();
		ProjectileLiftTimeData.ImpactTime = GetWorld()->GetRealTimeSeconds();
		ProjectileLiftTimeData.ImpactVelocity = GetVelocity();
		UE_LOG(LogINSProjectile
		       , Log
		       , TEXT("Projectile hit something,this hit component name:%s,hit other component name:%s,other actor name :%s")
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
			ProjectileLiftTimeData.ImpactPlayer  = HitCharacter->GetController();
			ProjectileLiftTimeData.EndLoc = GetActorLocation();
		}
		UE_LOG(LogINSProjectile, Log, TEXT("Projectile%s Hit Happened,Will Force Send a Movement info to all Conected Clients"), *GetName());
		if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
		{
			OnRep_HitCounter();
		}
		bIsProcessingHit = false;
		//CalAndSpawnPenetrateProjectile(Hit, GetVelocity());
		SetLifeSpan(0.1f);
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
	if (HasAuthority())
	{
		FRepMovement* const RepMovement = (FRepMovement*)&GetReplicatedMovement();
		RepMovement->LocationQuantizationLevel = MovementQuantizeLevel;
		RepMovement->RotationQuantizationLevel = ERotatorQuantization::ByteComponents;
		RepMovement->VelocityQuantizationLevel = MovementQuantizeLevel;
	}
	if (!bVisualProjectile)
	{
		TracerParticle->DestroyComponent(true);
		ProjectileMesh->DestroyComponent(true);
		SetActorHiddenInGame(true);
		if (IsNetMode(NM_ListenServer) || IsNetMode(NM_Standalone))
		{
			InitClientFakeProjectile(this);
		}
	}
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, TEXT("OnProjectileHit"));
		CollisionComp->OnComponentHit.AddUnique(Delegate);
	}
	if (GetIsFakeProjectile())
	{
		TracerPaticleSizeTickFun.bCanEverTick = true;
		TracerPaticleSizeTickFun.SetTickFunctionEnable(true);
		TracerPaticleSizeTickFun.bTickEvenWhenPaused = true;
		TracerPaticleSizeTickFun.RegisterTickFunction(GetLevel());
		TracerPaticleSizeTickFun.Target = this;
	}
	if (GetVisualFakeProjectile())
	{
		if (GetVisualFakeProjectile()->CurrentPenetrateCount > 1)
		{
			GetProjectileMesh()->SetHiddenInGame(true);
			GetWorldTimerManager().SetTimer(FakeProjVisibilityTimer, this, &AINSProjectile::EnableFakeProjVisual, 1.f, false, 0.1f);
		}
		else
		{
			//hide this fake projectile for one frame-time
			GetProjectileMesh()->SetHiddenInGame(false);
		}
	}
	UNetDriver* NetDriver = GetNetDriver();
	if (NetDriver != nullptr && NetDriver->IsServer())
	{
		InitRepTickFunc.Target = this;
		InitRepTickFunc.RegisterTickFunction(GetLevel());
	}
	if(!bVisualProjectile)
	{
		ProjectileLiftTimeData.StartLoc = GetActorLocation();
	}
}

void AINSProjectile::LifeSpanExpired()
{
	Super::LifeSpanExpired();
	ProjectileLiftTimeData.EndLoc = GetActorLocation();
	if(!bVisualProjectile)
	{
		UE_LOG(LogINSProjectile, Log, TEXT("Projectile:%s has reached it's life span,life span data is: %s"), *GetName(),*ProjectileLiftTimeData.ToString());
	}
}

void AINSProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	GetProjectileMovementComp()->SetOwnerProjectile(this);
	ProjectileLiftTimeData.InitialSpeed = GetProjectileMovementComp()->InitialSpeed;
	ProjectileLiftTimeData.StartTime = GetWorld()->GetRealTimeSeconds();
}

void AINSProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	Super::PostNetReceiveVelocity(NewVelocity);
}

void AINSProjectile::PostNetReceiveLocationAndRotation()
{
	Super::PostNetReceiveLocationAndRotation();
	if (!bVisualProjectile)
	{
		SetActorLocationAndRotation(GetReplicatedMovement().Location, GetReplicatedMovement().Rotation);
	}
	else
	{
		if (GetActorLocation().Equals(GetReplicatedMovement().Location, 10.f))
		{
			bNeedPositionSync = false;
			bInPositionSync = true;
		}
	}
}

void AINSProjectile::GatherCurrentMovement()
{
		if (HasAuthority()&&!bVisualProjectile && RootComponent != nullptr && !IsPendingKill() && !IsPendingKillPending())
		{
			// If we are attached, don't replicate absolute position
			if (RootComponent->GetAttachParent() != nullptr)
			{
				Super::GatherCurrentMovement();
			}
			else
			{
				FRepMovement OptRepMovement;
				OptRepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(RootComponent->GetComponentLocation(), this);
				OptRepMovement.Rotation = RootComponent->GetComponentRotation();
				OptRepMovement.LinearVelocity = GetVelocity();
				OptRepMovement.AngularVelocity = FVector::ZeroVector;
				OptRepMovement.bRepPhysics = true;
				OptRepMovement.LocationQuantizationLevel = MovementQuantizeLevel;
				//OptRepMovement.RotationQuantizationLevel = ERotatorQuantization::ShortComponents;
				OptRepMovement.VelocityQuantizationLevel = MovementQuantizeLevel;
				SetReplicatedMovement(OptRepMovement);
				bForceMovementReplication = false;
				bIsGatheringMovement = false;
				LastMovementRepTime = GetWorld()->GetRealTimeSeconds();
			}
		}
	// if (!bVisualProjectile)
	// {
	// 	Super::GatherCurrentMovement();
	// }
}


void AINSProjectile::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
		if (HasAuthority() && !bIsGatheringMovement && !bVisualProjectile&& IsReplicatingMovement())
		{
			const bool bRepDeltaTimeAllowed = GetWorld()->GetRealTimeSeconds() - LastMovementRepTime >= MovementRepInterval;
			if (bRepDeltaTimeAllowed || bForceMovementReplication)
			{
				TGuardValue<bool> HitGuard(bIsGatheringMovement, true);
				GatherCurrentMovement();
			}
		}
	//Super::PreReplication(ChangedPropertyTracker);
}

void AINSProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSProjectile, HitCounter);
	DOREPLIFETIME(AINSProjectile, OwnerWeapon);
	DOREPLIFETIME(AINSProjectile, CurrentPenetrateCount);
	DOREPLIFETIME(AINSProjectile, bScanTraceProjectile);
	DOREPLIFETIME_CONDITION(AINSProjectile, MovementQuantizeLevel, COND_InitialOnly);
}

void AINSProjectile::OnRep_ReplicatedMovement()
{
	FRepMovement DecompressedMovementRep = GetReplicatedMovement();
	float QuantizationDivider = 1.f;
	if(MovementQuantizeLevel==EVectorQuantization::RoundOneDecimal)
	{
		QuantizationDivider = 10.f;
	}
	if(MovementQuantizeLevel==EVectorQuantization::RoundTwoDecimals)
	{
		QuantizationDivider = 100.f;
	}
	DecompressedMovementRep.Location = DecompressedMovementRep.Location / QuantizationDivider;
	DecompressedMovementRep.LinearVelocity = DecompressedMovementRep.LinearVelocity / QuantizationDivider;
	SetReplicatedMovement(DecompressedMovementRep);
	Super::OnRep_ReplicatedMovement();
	if (GetVisualFakeProjectile())
	{
		GetVisualFakeProjectile()->SetReplicatedMovement(GetReplicatedMovement());
		GetVisualFakeProjectile()->OnRep_ReplicatedMovement();
	}
}

void AINSProjectile::InitClientFakeProjectile(const AINSProjectile* const ServerNetProjectile)
{
	const FTransform SpawnTransform(GetActorRotation(), GetActorLocation(), FVector::OneVector);
	VisualFakeProjectile = GetWorld()->SpawnActorDeferred<AINSProjectile>(this->GetClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (VisualFakeProjectile)
	{
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		GetVisualFakeProjectile()->bUsingDebugTrace = false;
#endif
		GetVisualFakeProjectile()->SetIsFakeProjectile(true);
		GetVisualFakeProjectile()->NetAuthorityProjectile = this;
		GetVisualFakeProjectile()->SetOwnerWeapon(GetOwnerWeapon());
		GetVisualFakeProjectile()->SetCurrentPenetrateCount(ServerNetProjectile->GetCurrentPenetrateCount() + 1);
		GetVisualFakeProjectile()->GetCollsioncomp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetVisualFakeProjectile()->GetCollsioncomp()->SetUseCCD(false);
		GetVisualFakeProjectile()->GetCollsioncomp()->SetAllUseCCD(false);
		GetVisualFakeProjectile()->GetProjectileMovementComp()->InitialSpeed = OwnerWeapon->GetMuzzleSpeedValue();
		GetVisualFakeProjectile()->SetScanTraceTime(OwnerWeapon->GetScanTraceRange() / OwnerWeapon->GetMuzzleSpeedValue());
		UGameplayStatics::FinishSpawningActor(VisualFakeProjectile, SpawnTransform);
	}
}

void AINSProjectile::SendInitialReplication()
{
	UE_LOG(LogINSProjectile, Log, TEXT("start send a initial bunch replocation %s manually to relavant Clients"), *GetName());
	UNetDriver* const NetDriver = GetNetDriver();
	if (NetDriver != nullptr && NetDriver->IsServer() && !IsPendingKillPending() && (ProjectileMoveComp->Velocity.Size() >= 7500.0f))
	{
		NetDriver->ReplicationFrame++;
		for (int32 i = 0; i < NetDriver->ClientConnections.Num(); i++)
		{
			if (NetDriver->ClientConnections[i]->State == USOCK_Open && NetDriver->ClientConnections[i]->PlayerController != nullptr && NetDriver->ClientConnections[i]->IsNetReady(0))
			{
				const AActor* ViewTarget = NetDriver->ClientConnections[i]->PlayerController->GetViewTarget();
				if (ViewTarget == nullptr)
				{
					ViewTarget = NetDriver->ClientConnections[i]->PlayerController;
				}
				FVector ViewLocation = ViewTarget->GetActorLocation();
				{
					FRotator ViewRotation = NetDriver->ClientConnections[i]->PlayerController->GetControlRotation();
					NetDriver->ClientConnections[i]->PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
				}
				// Workaround to skip deprecation warning where it calls the PlayerController version of this function
				if (IsNetRelevantFor(NetDriver->ClientConnections[i]->PlayerController, ViewTarget, ViewLocation))
				{
					UActorChannel* Channel = NetDriver->ClientConnections[i]->FindActorChannelRef(this);
					if (Channel == nullptr)
					{
						if (NetDriver->ClientConnections[i]->GetClientWorldPackageName() == GetWorld()->GetOutermost()->GetFName() && NetDriver->ClientConnections[i]->ClientHasInitializedLevelFor(this))
						{
							Channel = Cast<UActorChannel>(NetDriver->ClientConnections[i]->CreateChannelByName(NAME_Actor, EChannelCreateFlags::OpenedLocally, INDEX_NONE));
							if (Channel != nullptr)
							{
								Channel->SetChannelActor(this, ESetChannelActorFlags::None);
							}
						}
					}
					if (Channel != nullptr && Channel->OpenPacketId.First == INDEX_NONE)
					{
						if (!Channel->bIsReplicatingActor)
						{
							bForceMovementReplication = true;
							GatherCurrentMovement();
							Channel->ReplicateActor();
							UE_LOG(LogINSProjectile, Log, TEXT("send a initial bunch replocation %s manually to  all relavent Clients done"), *GetName());
						}
					}
				}
			}
		}
	}
}

void AINSProjectile::CheckImpactHit()
{
	FVector ProjectileLoc(ForceInit);
	FVector ProjectileDir(ForceInit);
	ProjectileLoc = GetReplicatedMovement().Location;
	ProjectileDir = GetReplicatedMovement().Rotation.Vector();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (VisualFakeProjectile)
	{
		QueryParams.AddIgnoredActor(VisualFakeProjectile);
	}
	QueryParams.AddIgnoredActor(GetOwnerWeapon());
	QueryParams.AddIgnoredActor(GetOwnerWeapon() == nullptr ? nullptr : GetOwnerWeapon()->GetOwnerCharacter());
	QueryParams.bReturnPhysicalMaterial = true;
	const FVector TraceStart = ProjectileLoc - ProjectileDir * 100.f;
	const float TraceRange = 500.f;
	const FVector TraceEnd = TraceStart + ProjectileDir * TraceRange;
	FHitResult ImpactHit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(ImpactHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Camera, QueryParams);
	const AActor* HitActor = ImpactHit.Actor.Get();
	UClass* HitActorClass = HitActor == nullptr ? nullptr : HitActor->GetClass();
	if (ImpactHit.bBlockingHit)
	{
		if (VisualFakeProjectile)
		{
			VisualFakeProjectile->bDelayedHit = false;
		}
		if (HitActor && HitActorClass && !HitActorClass->IsChildOf(AINSCharacter::StaticClass()) && !HitActorClass->IsChildOf(AINSWeaponBase::StaticClass()))
		{
			if (GetVisualFakeProjectile())
			{
				GetVisualFakeProjectile()->GetProjectileMovementComp()->StopMovementImmediately();
			}
			FTransform EffectActorTransform(ImpactHit.ImpactNormal.ToOrientationRotator(), ImpactHit.ImpactPoint, FVector::OneVector);
			AINSImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AINSImpactEffect>(PointImapactEffectsClass, EffectActorTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (EffectActor)
			{
				EffectActor->SetImpactHit(ImpactHit);
				UGameplayStatics::FinishSpawningActor(EffectActor, EffectActorTransform);
			}
		}
		if (VisualFakeProjectile)
		{
			VisualFakeProjectile->Destroy();
		}
	}
	if (bDelayedHit)
	{
		GetVisualFakeProjectile()->Destroy();
	}
	else
	{
		if (GetVisualFakeProjectile())
		{
			GetVisualFakeProjectile()->bDelayedHit = true;
		}
	}
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	if (bUsingDebugTrace)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 3.0f);
	}
#endif
}

void AINSProjectile::SetMuzzleSpeed(float NewSpeed)
{
	ProjectileMoveComp->InitialSpeed = NewSpeed;
}

void AINSProjectile::SetScanTraceTime(float NewTime)
{
	ScanTraceTime = NewTime;
	UE_LOG(LogINSProjectile, Log, TEXT("Projectile named %s will travel with a scan trace time %f"), *GetName(), ScanTraceTime);
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
	if (bVisualProjectile)
	{
		if (bNeedPositionSync)
		{
			const float Distance = FVector::Distance(GetActorLocation(), GetReplicatedMovement().Location);
			SetActorLocationAndRotation(FMath::VInterpTo(GetActorLocation(), GetReplicatedMovement().Location, DeltaTime, Distance / 10.f), GetReplicatedMovement().Rotation);
			if (GetActorLocation().Equals(GetReplicatedMovement().Location, 1.f))
			{
				bNeedPositionSync = false;
			}
		}
	}
}

void AINSProjectile::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	if (&ThisTickFunction == &InitRepTickFunc)
	{
		SendInitialReplication();
		PrimaryActorTick.RemovePrerequisite(this, InitRepTickFunc);
		InitRepTickFunc.UnRegisterTickFunction();
	}
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (&ThisTickFunction == &TracerPaticleSizeTickFun && bVisualProjectile)
	{
		const FVector CurrentTracerScale = TracerParticle->GetComponentTransform().GetScale3D();
		const float CurrentScaleX = CurrentTracerScale.X;
		const float CurrentScaleY = CurrentTracerScale.Y;
		const float CurrentScaleZ = CurrentTracerScale.Z;
		const float UpdatedScaleX = FMath::FInterpTo(CurrentScaleX, 5.f, DeltaTime, 3.f);
		const float UpdateScaleY = FMath::FInterpTo(CurrentScaleY, 5.f, DeltaTime, 3.f);
		const float UpdateScaleZ = FMath::FInterpTo(CurrentScaleZ, 5.f, DeltaTime, 3.f);
		if (CurrentScaleX <= 20.f)
		{
			TracerParticle->SetWorldScale3D(FVector(UpdatedScaleX, UpdateScaleY, UpdateScaleZ));
		}
		else
		{
			TracerPaticleSizeTickFun.SetTickFunctionEnable(false);
			TracerPaticleSizeTickFun.UnRegisterTickFunction();
		}
	}
}
