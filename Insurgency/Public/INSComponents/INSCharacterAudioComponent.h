// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Insurgency/Insurgency.h"
#include "INSCharacterAudioComponent.generated.h"

class UINSCharVoiceAssetData;
/**
 *
 */
UCLASS()
class INSURGENCY_API UINSCharacterAudioComponent : public UAudioComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		UINSCharVoiceAssetData* MaleVoiceData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceData")
		UINSCharVoiceAssetData* FeMaleVoiceData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VoiceType")
		EVoiceType CurrentVoiceType;
public:
	virtual void SetVoiceType(EVoiceType NewVoiceType);

	virtual void PlayVoice();

	virtual class USoundCue* GetSoundToPlay();
};
