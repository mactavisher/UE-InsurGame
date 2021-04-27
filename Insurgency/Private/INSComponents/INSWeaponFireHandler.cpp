// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSWeaponFireHandler.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
UINSWeaponFireHandler::UINSWeaponFireHandler(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsToFire = false;
	bWantsToFire = false;
	ShotTimeRemaining = -0.01f;
	FireInterval = 0.1f;
	LastFireTime = 0.f;
}

void UINSWeaponFireHandler::BeginPlay()
{
	Super::BeginPlay();
}

void UINSWeaponFireHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ClearFiring();
}

void UINSWeaponFireHandler::SetOwnerWeapon(class AINSWeaponBase* NewWeapon)
{
	this->OwnerWeapon = NewWeapon;
}

void UINSWeaponFireHandler::BeginWeaponFire(enum EWeaponFireMode NewFireMode)
{
	this->CurrentFireMode = NewFireMode;
	if (CheckCanFireAgian())
	{
		if (CurrentFireMode == EWeaponFireMode::SEMI)
		{
			GetWorld()->GetTimerManager().SetTimer(SemiFireTimer, this, &UINSWeaponFireHandler::FireShot, FireInterval, false, 0.f);
		}
		else if (CurrentFireMode == EWeaponFireMode::SEMIAUTO)
		{
			GetWorld()->GetTimerManager().SetTimer(SemiAutoFireTimer, this, &UINSWeaponFireHandler::FireShot, FireInterval * 0.8f, true, 0.f);
		}
		else if (CurrentFireMode == EWeaponFireMode::FULLAUTO)
		{
			GetWorld()->GetTimerManager().SetTimer(FullAutoFireTimer, this, &UINSWeaponFireHandler::FireShot, FireInterval, true, 0.f);
		}
	}
}

void UINSWeaponFireHandler::StopWeaponFire()
{
	// semi auto fire will go until it finish it's round , won't release manually
	GetWorld()->GetTimerManager().ClearTimer(SemiFireTimer);
	GetWorld()->GetTimerManager().ClearTimer(FullAutoFireTimer);
	OwnerWeapon->SetWeaponState(EWeaponState::IDLE);
}

bool UINSWeaponFireHandler::CheckCanFireAgian()
{
	return OwnerWeapon && OwnerWeapon->CheckCanFire()&&GetWorld()->GetTimeSeconds() - LastFireTime >= FireInterval;
}

void UINSWeaponFireHandler::RefireAndCheckTimer()
{
	if (CheckCanFireAgian())
	{
		OwnerWeapon->FireShot();
		ShotTimeRemaining += FireInterval;
	}
}

void UINSWeaponFireHandler::FireShot()
{
	OwnerWeapon->FireShot();
	LastFireTime = GetWorld()->GetTimeSeconds();
	OwnerWeapon->SetWeaponState(EWeaponState::FIRING);
	bIsFiring = true;
	if (CurrentFireMode == EWeaponFireMode::SEMIAUTO)
	{
		SemiAutoCount += (uint8)1;
	}
	if (OwnerWeapon->CurrentClipAmmo == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(SemiAutoFireTimer);
		GetWorld()->GetTimerManager().ClearTimer(FullAutoFireTimer);
	}
	if ((SemiAutoCount == 3))
	{
		GetWorld()->GetTimerManager().ClearTimer(SemiAutoFireTimer);
		//reset the semi auto count
		SemiAutoCount = (uint8)0;
	}
	GetWorld()->GetTimerManager().ClearTimer(SemiFireTimer);
}

void UINSWeaponFireHandler::ClearFiring()
{
	if (bIsFiring) 
	{
		if (GetWorld()->GetTimeSeconds() - LastFireTime >= FireInterval * 0.6f)
		{
			OwnerWeapon->SetWeaponState(EWeaponState::IDLE);
			bIsFiring = false;
		}
	}
}
