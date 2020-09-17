// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "Engine/Canvas.h"
#include "INSDamageType.generated.h"


struct FCanvasIcon;

/**
 *
 */
UCLASS()
class INSURGENCY_API UINSDamageType : public UDamageType
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Message")
		FText DamageMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Message")
		FText DeathMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Message")
		FText FallingDamageMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Message")
		FText FallingDeathMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Message")
		FCanvasIcon DamageIcon;
};
