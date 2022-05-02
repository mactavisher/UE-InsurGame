// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSProjectileMovementComponent.h"
#include "INSProjectiles/INSProjectile.h"

UINSProjectileMovementComponent::UINSProjectileMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ProjectileGravityScale = 0.f;
	OwnerProjectile = nullptr;
	bScanTraceProjectile = false;
	bScanTraceCondSet = false;
	ScanTraceMoveTime = 0.f;
	CurrentScanTraceMoveTime = 0.f;
	//PrimaryComponentTick.bCanEverTick = false;
	//bRotationFollowsVelocity = true;
}

void UINSProjectileMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//both server and client fake projectile should tick on there own,but will make the client one to match the server one
	if (bScanTraceCondSet && ScanTraceMoveTime > 0.f)
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		if (!bScanTraceProjectile)
		{
			CurrentScanTraceMoveTime += DeltaTime;
			if (CurrentScanTraceMoveTime > ScanTraceMoveTime)
			{
				Velocity *= 0.99f;
				ProjectileGravityScale = FMath::Clamp<float>(DeltaTime * 90.f, ProjectileGravityScale, 20.f);
			}
		}
	}
}

void UINSProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	EnableTick(true);
}

void UINSProjectileMovementComponent::SetScanTraceProjectile(bool bFromScanTrace)
{
	bScanTraceProjectile = bFromScanTrace;
	bScanTraceCondSet = true;
}

void UINSProjectileMovementComponent::EnableTick(const bool bEnableTick)
{
	PrimaryComponentTick.bCanEverTick = bEnableTick;
	PrimaryComponentTick.SetTickFunctionEnable(bEnableTick);
}

void UINSProjectileMovementComponent::SetScanTraceMoveTime(const float ScanTraceTime)
{
	ScanTraceMoveTime = ScanTraceTime;
}
