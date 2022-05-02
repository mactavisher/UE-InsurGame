// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAssets/INSWeaponAssets.h"
#include "INSItems/INSItems.h"

void FWeaponInfoData::CopyDataFromTable(FItemTableRow* ItemTableRow)
{
	Super::CopyDataFromTable(ItemTableRow);
	const FWeaponTableRow* WeaponTableRow = (FWeaponTableRow*)ItemTableRow;
	if (WeaponTableRow)
	{
		WeaponType = WeaponTableRow->WeaponType;
		Priority = WeaponTableRow->Priority;
		BaseClipCapacity = WeaponTableRow->BaseClipCapacity;
		BaseClipSize = WeaponTableRow->BaseClipSize;
		TimeBetweenShots = WeaponTableRow->TimeBetweenShots;
		MuzzleVelocity = WeaponTableRow->MuzzleVelocity;
		BaseDamage = WeaponTableRow->BaseDamage;
		ScanTraceRange = WeaponTableRow->ScanTraceRange;
		bNeedOpticRail = WeaponTableRow->bNeedOpticRail;
		BaseAimTime = WeaponTableRow->BaseAimTime;
		RecoilHorizontalBase = WeaponTableRow->RecoilHorizontalBase;
		RecoilVerticalBase = WeaponTableRow->RecoilVerticalBase;
	}

	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("copy weapon info failed!Trying to copy weapon info from a non-weapon row type table!"));
	}
}
