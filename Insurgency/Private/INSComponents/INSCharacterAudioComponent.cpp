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
		USoundCue* SelectedSoundToPlay = bIsTeamDamage ? GetSoundToPlay(EVoiceType::TAKE_TEAM_DAMAGE) : GetSoundToPlay(EVoiceType::TAKE_DAMAGE);
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


void UINSCharacterAudioComponent::OnCauseDamage(bool bTeamDamage, bool bVictimDead)
{
	USoundCue* SelectedSound = nullptr;
	/*if (bTeamDamage)
	{
		SelectedSound = GetSoundToPlay(EVoiceType::CAUSE_FRIENDLY_DAMAGE);
	}*/
	if (bVictimDead)
	{
		SelectedSound = GetSoundToPlay(EVoiceType::KILL_PLAYER);
	}
	Stop();
	SetSound(SelectedSound);
	Play();
}

void UINSCharacterAudioComponent::SetOwnerCharacter(class AINSCharacter* NewCharacter)
{
	OwnerCharacter = NewCharacter;
}

bool UINSCharacterAudioComponent::GetIsOwnerCharacterDead() const
{
	if (GetOwnerCharacter())
	{
		return GetOwnerCharacter()->GetIsDead();
	}
	return false;
}

class USoundCue* UINSCharacterAudioComponent::GetSoundToPlay(const EVoiceType NewVoiceType)
{
	USoundCue* SoundToPlay = nullptr;
	switch (NewVoiceType)
	{
	case EVoiceType::TAKE_DAMAGE:SoundToPlay = MaleVoiceData->MaleVoiceData.TakeDamageVoice; break;
	case EVoiceType::DIE:SoundToPlay = MaleVoiceData->MaleVoiceData.DieVoice; break;
	case EVoiceType::RELOADING:SoundToPlay = MaleVoiceData->MaleVoiceData.ReloadWeapon; break;
	case EVoiceType::TAKE_TEAM_DAMAGE:SoundToPlay = MaleVoiceData->MaleVoiceData.TeamDamageVoice; break;
	case EVoiceType::KILL_PLAYER:SoundToPlay = MaleVoiceData->MaleVoiceData.KillEnemy; break;
	case EVoiceType::CAUSE_FFIENDLY_KILL:SoundToPlay = MaleVoiceData->MaleVoiceData.FriendlyDamage;break;
	default:SoundToPlay = nullptr; break;
	}
	if (SoundToPlay == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to play sound with no sound cue"));
	}
	return SoundToPlay;
}
