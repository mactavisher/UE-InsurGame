// Fill out your copyright notice in the Description page of Project Settings.


#include "INSGameModes/INSGameMode_Free_For_All.h"

void AINSGameMode_Free_For_All::BeginPlay()
{
	Super::BeginPlay();
}

bool AINSGameMode_Free_For_All::GetIsTeamDamage(class AController* DamageInstigator, class AController* Victim)
{
	// DO NOT call super here
	return false;
}
