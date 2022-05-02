// Fill out your copyright notice in the Description page of Project Settings.


#include "INSProjectiles/INSProjectileBase.h"
#include "INSComponents/INSProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Net/UnrealNetwork.h"

AINSProjectileBase::AINSProjectileBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	bReplicates = true;
	CollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("CollisionComp"));
	CollisionComp->SetCollisionProfileName(FName(TEXT("Projectile")));
	RootComponent = CollisionComp;
	CollisionComp->SetSphereRadius(1.f);
	ProjectileMovementComponent = ObjectInitializer.CreateDefaultSubobject<UINSProjectileMovementComponent>(this, TEXT("ProjectileMovementComp"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComp;
	ProjectileMovementComponent->SetOwnerProjectile(this);
	OwnerWeapon = nullptr;
	bScanTraceConditionSet = false;
	bScanTraceProjectile = false;
	InitialLifeSpan = 10.f;
}

// Called when the game starts or when spawned
void AINSProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	//CollisionComp->OnComponentHit.AddDynamic(this,&AINSProjectileBase::OnProjectileHit);
}

void AINSProjectileBase::OnRep_ScanTraceCondition()
{
	bScanTraceConditionSet = true;
}

void AINSProjectileBase::OnRep_OwnerWeapon()
{
}

void AINSProjectileBase::OnRep_ScanTraceTime()
{
}

void AINSProjectileBase::BindingHitDelegate(bool bDoBinding)
{
	if (bDoBinding)
	{
		HitDelegate.BindUFunction(this, TEXT("OnProjectileHit"));
		CollisionComp->OnComponentHit.AddUnique(HitDelegate);
	}
	if (!bDoBinding && HitDelegate.IsBound())
	{
		HitDelegate.Unbind();
		CollisionComp->OnComponentHit.RemoveAll(this);
	}
}

void AINSProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	CollisionComp->OnComponentHit.Remove(HitDelegate);
}

void AINSProjectileBase::SetIsScanTraceProjectile(const bool bScanTrace)
{
	bScanTraceProjectile = bScanTrace;
	if (HasAuthority())
	{
		OnRep_ScanTraceCondition();
		//disable movement replication if we are scan trace projectile
		if (bScanTraceProjectile)
		{
			SetReplicatingMovement(false);
		}
	}
	ProjectileMovementComponent->SetScanTraceProjectile(bScanTrace);
}

void AINSProjectileBase::SetOwnerWeapon(AINSWeaponBase* NewOwnerWeapon)
{
	OwnerWeapon = NewOwnerWeapon;
	if (HasAuthority())
	{
		OnRep_OwnerWeapon();
	}
}

void AINSProjectileBase::SetMuzzleSpeed(float MuzzleSpeed)
{
	ProjectileMovementComponent->InitialSpeed = MuzzleSpeed;
	ProjectileMovementComponent->EnableTick(true);
}

void AINSProjectileBase::SetMoveIgnoredActor(TArray<AActor*> ActorsToIgnore)
{
	for (AActor* ToIgnore : ActorsToIgnore)
	{
		CollisionComp->MoveIgnoreActors.Add(ToIgnore);
	}
}

void AINSProjectileBase::SetProjectileTick(const bool bEnableTick)
{
	PrimaryActorTick.bCanEverTick = bEnableTick;
	PrimaryActorTick.SetTickFunctionEnable(bEnableTick);
}

void AINSProjectileBase::SetScanTraceTime(const float NewTime)
{
	ScanTraceMoveTime = NewTime;
}

void AINSProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AINSProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSProjectileBase, ScanTraceMoveTime);
}

void AINSProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CollisionComp->SetAllUseCCD(true);
	ProjectileMovementComponent->SetOwnerProjectile(this);
	if (HasAuthority())
	{
		BindingHitDelegate(true);
	}
}
