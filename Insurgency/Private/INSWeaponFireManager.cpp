// Fill out your copyright notice in the Description page of Project Settings.


#include "INSWeaponFireManager.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

UINSWeaponFireManager::UINSWeaponFireManager(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsToFire = false;
	bWantsToFire = false;
	ShotTimeRemaining = -0.01f;
}

void UINSWeaponFireManager::BeginPlay()
{
	Super::BeginPlay();
}

void UINSWeaponFireManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (OwnerWeapon&&ShotTimeRemaining<=0.f)
	{
		OwnerWeapon->FireShot();
	}
}

void UINSWeaponFireManager::SetOwnerWeapon(class AINSWeaponBase* NewWeapon)
{
	this->OwnerWeapon = NewWeapon;
}

void UINSWeaponFireManager::BeginFire(EWeaponFireMode CurrentFireMode)
{
	bWantsToFire = true;
	if (CheckCanFireAgian())
	{
		bIsFiring = true;
	}
}

void UINSWeaponFireManager::StopFire()
{
	bWantsToFire = false;
}

bool UINSWeaponFireManager::CheckCanFireAgian()
{
	return OwnerWeapon && OwnerWeapon->CheckCanFire();
}

