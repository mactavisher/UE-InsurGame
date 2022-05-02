// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSWeaponMeshComponent.h"
#include "INSAnimation/INSWeaponAnimInstance.h"

UINSWeaponMeshComponent::UINSWeaponMeshComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	SetAnimInstanceClass(UINSWeaponAnimInstance::StaticClass());
}

void UINSWeaponMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}

FTransform UINSWeaponMeshComponent::GetShellSpawnTransform() const
{
	FVector ShellSpawnLoc = GetSocketLocation(WeaponSockets.ShellEjectSocket);
	FRotator ShellSpawnRot = GetSocketRotation(WeaponSockets.ShellEjectSocket);
	return FTransform(ShellSpawnRot, ShellSpawnLoc, FVector(1.f, 1.f, 1.f));
}
