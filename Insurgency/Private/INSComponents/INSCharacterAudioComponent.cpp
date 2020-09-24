// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSCharacterAudioComponent.h"
#include "INSAssets/INSCharVoiceAssetData.h"
#include "Sound/SoundCue.h"
#include "INSCharacter/INSPlayerCharacter.h"

void UINSCharacterAudioComponent::SetVoiceType(EVoiceType NewVoiceType)
{
	CurrentVoiceType = NewVoiceType;
}


void UINSCharacterAudioComponent::OnTakeDamage(const bool bIsTeamDamage)
{
	if (!GetIsOwnerCharacterDead())
	{
		USoundCue* SelectedSoundToPlay = bIsTeamDamage ? GetSoundToPlay(EVoiceType::TEAMDAMAGE) : GetSoundToPlay(EVoiceType::TAKEDAMAGE);
		SetSound(SelectedSoundToPlay);
		Play();
	}
}

void UINSCharacterAudioComponent::OnWeaponReload()
{
	if (!GetIsOwnerCharacterDead())
	{
		USoundCue* SelectedSoundToPlay = GetSoundToPlay(EVoiceType::RELOADING);
		SetSound(SelectedSoundToPlay);
		Play();
	}
}

void UINSCharacterAudioComponent::OnManDown()
{

}

void UINSCharacterAudioComponent::OnDeath()
{
	USoundCue* SelectedSoundToPlay = GetSoundToPlay(EVoiceType::DIE);
	Stop();
	SetSound(SelectedSoundToPlay);
	Play();
}

void UINSCharacterAudioComponent::OnLowHeath()
{

}

void UINSCharacterAudioComponent::SetOwnerCharacter(class AINSCharacter* NewCharacter)
{
	OwnerCharacter = NewCharacter;
}

bool UINSCharacterAudioComponent::GetIsOwnerCharacterDead() const
{
	if (GetOwnerCharacter())
	{
		return GetOwnerCharacter()->GetIsCharacterDead();
	}
	return false;
}

class USoundCue* UINSCharacterAudioComponent::GetSoundToPlay(const EVoiceType NewVoiceType)
{
	USoundCue* SoundToPlay = nullptr;
	switch (NewVoiceType)
	{
	case EVoiceType::TAKEDAMAGE:SoundToPlay = MaleVoiceData->MaleVoiceData.TakeDamageVoice; break;
	case EVoiceType::DIE:SoundToPlay = MaleVoiceData->MaleVoiceData.DieVoice; break;
	case EVoiceType::RELOADING:SoundToPlay = MaleVoiceData->MaleVoiceData.ReloadWeapon; break;
	case EVoiceType::TEAMDAMAGE:SoundToPlay = MaleVoiceData->MaleVoiceData.TeamDamageVoice; break;
	default:SoundToPlay = nullptr; break;
	}
	if (SoundToPlay == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to play sound with no sound cue"));
	}
	return SoundToPlay;
}
