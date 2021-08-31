// Fill out your copyright notice in the Description page of Project Settings.


#include "INSDamageModifier/INSDamageModifier_Team.h"
#include "AIModule/Classes/AIController.h"
#include "INSGameplay/INSTeamInfo.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "INSGameModes/INSGameModeBase.h"
void UINSDamageModifier_Team::ModifyDamage(float& InDamage, FDamageEvent& DamageEvent, AController* Instigator, class AController* Victim)
{
	class AINSGameModeBase* const GM = Victim->GetWorld()->GetAuthGameMode<AINSGameModeBase>();
	if (GM)
	{
		const float OrigDamage = InDamage;
		const bool bIsTeamDamage = GM->GetIsTeamDamage(Instigator, Victim);
		if (bIsTeamDamage)
		{
			if (GM->GetAllowTeamDamage())
			{
				InDamage *= GM->GetTeamDamageModifier();
				UE_LOG(LogINSDamageModifier, Log, TEXT("Modify character%s damage by Team,modify damage value from:%f to:%f"), *(Victim->GetPawn()->GetName()), OrigDamage, InDamage);
			}
			else
			{
				InDamage = 0.f;
			}
			UE_LOG(LogINSDamageModifier, Log, TEXT("Modify character%s damage by Team, but team damage is not enabled,modify damage value from:%f to:%f"), *(Victim->GetPawn()->GetName()), OrigDamage, InDamage);

		}
	}
}
