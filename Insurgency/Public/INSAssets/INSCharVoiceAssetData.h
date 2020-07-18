// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "INSCharVoiceAssetData.generated.h"


class USoundCue;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FVoiceData {

	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* TakeDamageVoice;

	/** used to blend finger pull trigger */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* DieVoice;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* TeamDamageVoice;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* KillEnemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* ReloadWeapon;

	/** used to blend finger pull trigger */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* SeeEnemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* ThrowGranade;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* ThrowSmokingBang;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
	    USoundCue* Mandown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		USoundCue* FriendlyDamage;
};

UCLASS()
class INSURGENCY_API UINSCharVoiceAssetData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="MaleVoice")
	FVoiceData MaleVoiceData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FeMaleVoice")
	FVoiceData FemaleVoiceData;
};
