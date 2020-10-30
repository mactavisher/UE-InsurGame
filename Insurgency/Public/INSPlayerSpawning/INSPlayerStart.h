// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "INSPlayerStart.generated.h"

/**
 *
 */
UCLASS()
class INSURGENCY_API AINSPlayerStart : public APlayerStart
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		/** indicates if enable drawing a player start point debug sphere */
		uint8 bShowPlayerStartOverlapDebugSphere : 1;
#endif

protected:
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	/**
	 * draw a debug sphere to show player overlap sphere for debug purpose
	 */
	virtual void DrawPlayerStartPointOverlapSphere();
#endif

};
