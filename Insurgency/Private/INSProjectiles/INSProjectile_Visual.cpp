// Fill out your copyright notice in the Description page of Project Settings.


#include "INSProjectiles/INSProjectile_Visual.h"
#include "Components/SphereComponent.h"
#include "INSComponents/INSProjectileMovementComponent.h"
#include "INSEffects/INSImpactEffect.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

DEFINE_LOG_CATEGORY(LogVisualProjectile);
// Sets default values
AINSProjectile_Visual::AINSProjectile_Visual(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CollisionComp->bReturnMaterialOnMove = true;
	TracerParticleComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("TracerParticleComp"));
	TracerParticleComp->SetupAttachment(RootComponent);
	TracerParticleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TracerParticleComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	bReplicates = false;
	ServerProjectile = nullptr;
	SetReplicatingMovement(false);
	ServerProjectileTrans = FTransform::Identity;
}

void AINSProjectile_Visual::BeginPlay()
{
	Super::BeginPlay();
	RegisterProjectileHiddenTick(true);
}

void AINSProjectile_Visual::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AINSProjectile_Visual::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (&ThisTickFunction == &ProjectileHiddenTickFun)
	{
		const float Distance = GetDistanceTo(OwnerWeapon);
		const float HiddenRange = OwnerWeapon->GetMuzzleSpeedValue() / 1500.f;
		if (Distance >= HiddenRange)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green,TEXT("Enable visual projectile visibility"));
			SetActorHiddenInGame(false);
			RegisterProjectileHiddenTick(false);
		}
	}
}

void AINSProjectile_Visual::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SetActorHiddenInGame(true);
	//TracerParticleComp->AttachToComponent(RootComponent,FAttachmentTransformRules::SnapToTargetIncludingScale,NAME_None);
}

void AINSProjectile_Visual::SpawnImpactHit(const FHitResult& Hit)
{
	if (Hit.bBlockingHit)
	{
		const FTransform EffectActorTransform(Hit.ImpactNormal.ToOrientationRotator(), Hit.ImpactPoint, FVector::OneVector);
		AINSImpactEffect* const EffectActor = GetWorld()->SpawnActorDeferred<AINSImpactEffect>(
			ImpactEffectsClass, EffectActorTransform, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (EffectActor)
		{
			FHitResult ThisHit(Hit);
			EffectActor->SetImpactHit(ThisHit);
			UGameplayStatics::FinishSpawningActor(EffectActor, EffectActorTransform);
		}
	}
}

void AINSProjectile_Visual::RegisterProjectileHiddenTick(const bool bDoRegister)
{
	if (bDoRegister)
	{
		ProjectileHiddenTickFun.bCanEverTick = true;
		ProjectileHiddenTickFun.SetTickFunctionEnable(true);
		ProjectileHiddenTickFun.Target = this;
		ProjectileHiddenTickFun.RegisterTickFunction(GetLevel());
	}
	else
	{
		ProjectileHiddenTickFun.bCanEverTick = false;
		ProjectileHiddenTickFun.SetTickFunctionEnable(false);
		ProjectileHiddenTickFun.Target = nullptr;;
		ProjectileHiddenTickFun.UnRegisterTickFunction();
	}
}

void AINSProjectile_Visual::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                            const FHitResult& Hit)
{
	if (bScanTraceConditionSet && bScanTraceProjectile)
	{
		ProjectileMovementComponent->EnableTick(false);
		ProjectileMovementComponent->StopMovementImmediately();
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		const FHitResult ImpactHit(Hit);
		SpawnImpactHit(ImpactHit);
		Destroy();
	}
}

void AINSProjectile_Visual::OnServerProjectileHit(const FHitResult& HitResult)
{
	SpawnImpactHit(HitResult);
	Destroy(true);
}

void AINSProjectile_Visual::SetServerProjectile(AINSProjectile_Server* NewServerProjectile)
{
	ServerProjectile = NewServerProjectile;
}

void AINSProjectile_Visual::SetIsScanTraceProjectile(const bool bScanTrace)
{
	Super::SetIsScanTraceProjectile(bScanTrace);
	bScanTraceProjectile = bScanTrace;
	if (bScanTrace)
	{
		//if scan traced,client will handle the collision on it's own
		CollisionComp->SetCollisionProfileName(FName(TEXT("Projectile")));
	}
	else
	{
		//other wise collision should be handled on server side and get replicates
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}


void AINSProjectile_Visual::ReceiveServerProjectileHit()
{
	// do location check;
}

void AINSProjectile_Visual::SetScanTraceTime(const float NewTime)
{
	Super::SetScanTraceTime(NewTime);
	ProjectileMovementComponent->SetScanTraceMoveTime(NewTime);
}

void AINSProjectile_Visual::ReceiveServerProjectileTransform(FTransform& InServerProjectileTrans)
{
}
