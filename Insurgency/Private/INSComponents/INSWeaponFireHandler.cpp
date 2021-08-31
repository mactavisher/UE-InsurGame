// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSWeaponFireHandler.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSCharacter/INSPlayerCharacter.h"

UINSWeaponFireHandler::UINSWeaponFireHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsToFire = false;
	bWantsToFire = false;
	ShotTimeRemaining = -0.01f;
	FireInterval = 0.1f;
	LastFireTime = 0.f;
	bIsFiring = false;
	SemiAutoCount = 0;
	OwnerWeapon = nullptr;
}

void UINSWeaponFireHandler::BeginPlay()
{
	Super::BeginPlay();
}

void UINSWeaponFireHandler::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
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
	if (CheckCanFireAgain())
	{
		if (CurrentFireMode == EWeaponFireMode::SEMI)
		{
			FireShot();
		}
		else if (CurrentFireMode == EWeaponFireMode::SEMIAUTO)
		{
			GetWorld()->GetTimerManager().SetTimer(SemiAutoFireTimer, this, &UINSWeaponFireHandler::FireShot,OwnerWeapon->GetTimeBetweenShots() * 0.8f, true, 0.f);
		}
		else if (CurrentFireMode == EWeaponFireMode::FULLAUTO)
		{
			GetWorld()->GetTimerManager().SetTimer(FullAutoFireTimer, this, &UINSWeaponFireHandler::FireShot,OwnerWeapon->GetTimeBetweenShots(), true, 0.f);
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

bool UINSWeaponFireHandler::CheckCanFireAgain()
{
	return OwnerWeapon
		&& OwnerWeapon->CheckCanFire()
		&& GetWorld()->GetTimeSeconds() - LastFireTime >= OwnerWeapon->GetTimeBetweenShots();
}


void UINSWeaponFireHandler::FireShot()
{
	//get the base fire location
	FVector FireLoc(ForceInit);
	OwnerWeapon->GetBarrelStartLoc(FireLoc);

	//get the fire shot dir
	FVector FireDir(ForceInit);
	OwnerWeapon->GetFireDir(FireDir);

	//add weapon fire spread
	FVector SpreadDir(ForceInit);
	OwnerWeapon->ApplyWeaponSpread(SpreadDir, FireDir);
	if (OwnerWeapon)
	{
		if (OwnerWeapon->GetLocalRole() == ROLE_Authority)
		{
			OwnerWeapon->FireShot(FireLoc, SpreadDir.Rotation());
		}
		else if (OwnerWeapon->GetLocalRole() == ROLE_AutonomousProxy)
		{
			OwnerWeapon->ServerFireShot(FireLoc, SpreadDir.Rotation());
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
	OwnerWeapon->SetWeaponState(EWeaponState::FIRING);
	bIsFiring = true;
	if (CurrentFireMode == EWeaponFireMode::SEMIAUTO)
	{
		SemiAutoCount += static_cast<uint8>(1);
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
		SemiAutoCount = static_cast<uint8>(0);
	}
	GetWorld()->GetTimerManager().ClearTimer(SemiFireTimer);
}

void UINSWeaponFireHandler::ClearFiring()
{
	if (bIsFiring)
	{
		if (GetWorld()->GetTimeSeconds() - LastFireTime >= OwnerWeapon->GetTimeBetweenShots() * 0.5f)
		{
			if (OwnerWeapon->HasAuthority())
			{
				OwnerWeapon->SetWeaponState(EWeaponState::IDLE);
			}
			else if (OwnerWeapon->GetLocalRole() == ROLE_AutonomousProxy)
			{
				OwnerWeapon->ServerSetWeaponState(EWeaponState::IDLE);
			}
			bIsFiring = false;
		}
	}
}
