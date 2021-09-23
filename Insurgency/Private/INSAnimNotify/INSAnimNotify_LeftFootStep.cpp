// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimNotify/INSAnimNotify_LeftFootStep.h"
#include "INSComponents/INSCharSkeletalMeshComponent.h"
#include "INSEffects/INSImpactEffect.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UINSAnimNotify_LeftFootStep::UINSAnimNotify_LeftFootStep()
{
	//Default FootSocket Name
	FootSocketName = FName(TEXT("RightFootstep"));

#if WITH_EDITORONLY_DATA
	bShowDebugInfo = true;
#endif
}

FString UINSAnimNotify_LeftFootStep::GetNotifyName_Implementation() const
{
	if (FootImpactEffectClass)
	{
		return FootImpactEffectClass->GetName();
	}
	else
	{
		return Super::GetNotifyName_Implementation();
	}
}

void UINSAnimNotify_LeftFootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{

	FHitResult LeftFootStepHit(ForceInit);
	const FVector FootTraceStartLocation = MeshComp->GetSocketLocation(FootSocketName);
	const float TraceRange = 20.f;
	const FVector FootTraceEndLocation(FootTraceStartLocation + MeshComp->GetSocketRotation(FootSocketName).Vector() * TraceRange);
	//use the current mesh's world,otherwise nothing will happen
	MeshComp->GetWorld()->LineTraceSingleByChannel(LeftFootStepHit, FootTraceStartLocation, FootTraceEndLocation, ECC_Visibility);
	if (LeftFootStepHit.bBlockingHit)
	{
		const FTransform FootStepEffectSpawnTransform(FRotator::ZeroRotator, FootTraceEndLocation, FVector::OneVector);
		if (FootImpactEffectClass)
		{
			AINSImpactEffect* const FootStepImpactEffect = MeshComp->GetWorld()->SpawnActorDeferred<AINSImpactEffect>(FootImpactEffectClass,FootStepEffectSpawnTransform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			//set hit result to spawn the right Effects
			if (FootStepImpactEffect)
			{
				FootStepImpactEffect->SetImpactHit(LeftFootStepHit);
				UGameplayStatics::FinishSpawningActor(FootStepImpactEffect, FootStepEffectSpawnTransform);
			}
		}
	}

#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	if (bShowDebugInfo)
	{
		DrawDebugLine(MeshComp->GetWorld(), FootTraceStartLocation, FootTraceEndLocation, FColor::Red, false, 0.5f);
	}
#endif
}
