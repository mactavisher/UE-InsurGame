// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "INSTeamInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UINSTeamInterface : public UGenericTeamAgentInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INSURGENCY_API IINSTeamInterface : public IGenericTeamAgentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
