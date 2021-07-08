// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSProjectileMovementComponent.h"

UINSProjectileMovementComponent::UINSProjectileMovementComponent(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	ScanHitTime = 1.f;
	ProjectileGravityScale = 0.f;
	bScanTraceProjectile = false;
}

void UINSProjectileMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (OwnerProjectile&&OwnerProjectile->GetLocalRole()==ROLE_Authority)
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		if (!bScanTraceProjectile)
		{
			ScanHitTime += DeltaTime;
			if (ScanHitTime > OwnerProjectile->GetScanTraceTime())
			{
				Velocity *= 0.99f;
				ProjectileGravityScale = FMath::Clamp<float>(DeltaTime * 10.f, ProjectileGravityScale, 20.f);
			}
		}
	}
}

void UINSProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

