// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Insurgency/Insurgency.h"
#include "INSCharacterAudioComponent.generated.h"

class UINSCharVoiceAssetData;
class AINSCharacter;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
		AINSCharacter* OwnerCharacter;

	UPROPERTY()
	EVoiceType CurrentPlayingSoundType;

protected:
	virtual class USoundCue* GetSoundToPlay(const EVoiceType NewVoiceType);
	virtual void BeginPlay()override;

public:
	virtual void SetVoiceType(EVoiceType NewVoiceType);


	/**
	 * @desc Called when Character taking damage
	 * @param bIsTeamDamage   indicates if this damage is caused by Team
	 */
	virtual void OnTakeDamage(const bool bIsTeamDamage);

	/**
	 * called when Owner Character start reload it's Weapon
	 * @param bIsTeamDamage   indicates if this damage is caused by Team
	 */
	virtual void OnWeaponReload();

	/**
	 * called when Owner Character see team member down(death)
	 */
	virtual void OnManDown();

	/**
	 * called when Owner Character dead
	 */
	virtual void OnDeath();

	/**
	 * called when Owner Character is In low health
	 */
	virtual void OnLowHeath();

	/**
	 * called when Owner damages other character
	 */
	virtual void OnCauseDamage(bool bTeamDamage,bool bVictimDead);



	/**
	 * @desc returns the owner character of this audio comp
	 */
	inline virtual AINSCharacter* GetOwnerCharacter()const { return OwnerCharacter; }

	/**
	 * @desc set the OwnerCharacter of this audio comp;
	 * @Param NewCharacter the new Character to set for this comp
	 */
	virtual void SetOwnerCharacter(class AINSCharacter* NewCharacter);

	/**
	 * @desc returns the owner character is Dead or not
	 * @return bool   character's dead condition
	 */
	virtual bool GetIsPlayValid()const;

	UFUNCTION()
	virtual void OnSoundFinishPlay();
};
