// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSCharSkeletalMeshComponent.h"
#include "INSAnimation/INSCharacterAimInstance.h"

UINSCharSkeletalMeshComponent::UINSCharSkeletalMeshComponent(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	SetAnimInstanceClass(UINSCharacterAimInstance::StaticClass());
}