// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSPlayerCameraManager.h"

#include "INSCharacter/INSPlayerCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

AINSPlayerCameraManager::AINSPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ViewPitchMin = -70.f;
	ViewPitchMax = 65.f;
	bWantsToADS = false;
	ADSFov = 65.f;
	DefaultFOVBlendTime = 0.2f;
	DefaultFOV = 85.f;
	LockedFOV = DefaultFOV;
}

void AINSPlayerCameraManager::LimitViewPitch(FRotator& ViewRotation, float InViewPitchMin, float InViewPitchMax)
{
	Super::LimitViewPitch(ViewRotation, ViewPitchMin, ViewPitchMax);
}

void AINSPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	if (!IsNetMode(NM_DedicatedServer) && CurrentWeapon.Get())
	{
		Super::UpdateCamera(DeltaTime);
		FOVBlendSpeed = (DefaultFOV - CurrentWeapon.Get()->GetAimFOV()) / (CurrentWeapon.Get()->GetWeaponAimTime() == 0.f ? DefaultFOVBlendTime : CurrentWeapon.Get()->GetWeaponAimTime());
		if (bWantsToADS)
		{
			SetFOV(LockedFOV - DeltaTime * FOVBlendSpeed);
			if (LockedFOV <= ADSFov)
			{
				SetFOV(ADSFov);
			}
		}
		if (!bWantsToADS)
		{
			SetFOV(LockedFOV + DeltaTime * FOVBlendSpeed);
			if (LockedFOV >= DefaultFOV)
			{
				SetFOV(DefaultFOV);
			}
		}
	}
}

void AINSPlayerCameraManager::OnAim(float AimTime)
{
	bWantsToADS = true;
}

void AINSPlayerCameraManager::OnStopAim(float AimTime)
{
	bWantsToADS = false;
}

void AINSPlayerCameraManager::SetCurrentWeapon(class AINSWeaponBase* NewWeapon)
{
	CurrentWeapon = NewWeapon;
}
