// Fill out your copyright notice in the Description page of Project Settings.


#include "INSDamageModifier/INSDamageModifier_Bone.h"
#include "INSCharacter/INSCharacter.h"


void UINSDamageModifier_Bone::ModifyDamage(float& InDamage, FDamageEvent& DamageEvent, AController* Instigator, class AController* Victim)
{
	if (Victim)
	{
		AINSCharacter* VictimChar = Cast<AINSCharacter>(Victim->GetPawn());
		if (VictimChar)
		{
			FBoneDamageModifier BoneDamageModifier;
			VictimChar->GetBoneDamageModifierStruct(BoneDamageModifier);
			if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
			{
				FPointDamageEvent* PointDamageEvent = (FPointDamageEvent*)(&DamageEvent);
				if (PointDamageEvent)
				{
					const float BoneDamageModifierVal = BoneDamageModifier.GetBoneDamageModifier(PointDamageEvent->HitInfo.BoneName);
					const float OrigDamage = InDamage;
					InDamage *= BoneDamageModifierVal;
					UE_LOG(LogINSDamageModifier, Log, TEXT("Modify character%s damage by bone,modify damage value from:%f to:%f"), *VictimChar->GetName(), OrigDamage, InDamage);
				}
			}
		}
	}
}
