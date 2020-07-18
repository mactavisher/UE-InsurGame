// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSPlayerCameraManager.h"

AINSPlayerCameraManager::AINSPlayerCameraManager(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	ViewPitchMin = -70.f;
	ViewPitchMax = 70.f;
	bWantsToADS = false;
	ADSFov = 75.f;
	FOVBlendTime = 0.2f;
	LockedFOV = DefaultFOV;
	FOVBlendSpeed = (DefaultFOV - ADSFov) / (FOVBlendTime == 0.f ? 0.5f : FOVBlendTime);
}
void AINSPlayerCameraManager::LimitViewPitch(FRotator& ViewRotation, float InViewPitchMin, float InViewPitchMax)
{
	Super::LimitViewPitch(ViewRotation, ViewPitchMin, ViewPitchMax);
}

void AINSPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);
	FOVBlendSpeed = (DefaultFOV - ADSFov) / (FOVBlendTime == 0.f ? 0.5f : FOVBlendTime);
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

void AINSPlayerCameraManager::OnAim(float AimTime)
{
	bWantsToADS = true;
	FOVBlendTime = AimTime;
}

void AINSPlayerCameraManager::OnStopAim(float AimTime)
{
	bWantsToADS = false;
	FOVBlendTime = AimTime;
}
