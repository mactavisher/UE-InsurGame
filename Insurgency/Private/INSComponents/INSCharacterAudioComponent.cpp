// Fill out your copyright notice in the Description page of Project Settings.


#include "INSComponents/INSCharacterAudioComponent.h"
#include "INSAssets/INSCharVoiceAssetData.h"
#include "Sound/SoundCue.h"

void UINSCharacterAudioComponent::SetVoiceType(EVoiceType NewVoiceType)
{
	CurrentVoiceType = NewVoiceType;
}

void UINSCharacterAudioComponent::PlayVoice()
{
	if (!IsPlaying())
	{
		SetSound(GetSoundToPlay());
		Play(0.f);;
	}
}

class USoundCue* UINSCharacterAudioComponent::GetSoundToPlay()
{
	USoundCue* SoundToPlay = nullptr;
	switch (CurrentVoiceType)
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
