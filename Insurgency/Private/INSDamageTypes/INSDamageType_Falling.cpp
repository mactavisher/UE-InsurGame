// Fill out your copyright notice in the Description page of Project Settings.


#include "INSDamageTypes/INSDamageType_Falling.h"

UINSDamageType_Falling::UINSDamageType_Falling(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bCausedByWorld = true;
}