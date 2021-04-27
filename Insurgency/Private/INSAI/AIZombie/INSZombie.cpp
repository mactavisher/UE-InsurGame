// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAI/AIZombie/INSZombie.h"
#ifndef USkeletalMeshComponent
#include "Components/SkeletalMeshComponent.h"
#endif
#include "INSAnimation/INSZombieAnimInstance.h"
#include "INSAI/AIZombie/INSZombieController.h"
#include "Net/UnrealNetwork.h"
#ifndef UCapsuleComponent
#include "Components/CapsuleComponent.h"
#endif
#include "INSCharacter/INSPlayerCharacter.h"
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

DEFINE_LOG_CATEGORY(LogZombiePawn);

AINSZombie::AINSZombie(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	AttackDamage = 20.f;
	RagePoint = 0.f;
}

void AINSZombie::OnRep_Dead()
{
	Super::OnRep_Dead();
	GetMesh()->SetSimulatePhysics(true);
}

void AINSZombie::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AINSZombie, CurrentZombieMoveMode);
}

void AINSZombie::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ZombieController = Cast<AINSZombieController>(NewController);
	CachedZombieAnimInstance = Cast<UINSZombieAnimInstance>(GetMesh()->AnimScriptInstance);
}


float AINSZombie::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float DamageApplied = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ZombieController)
	{
		ZombieController->OnZombieTakeDamage(Damage, EventInstigator, DamageCauser);
	}
	return DamageApplied;
}

void AINSZombie::HandlesAttackRequest(class AINSZombieController* RequestZombieController)
{
	if (HasAuthority())
	{
		uint8 AvailableAttackNum = 3;
		uint8 SelectedAttackModeIndex = FMath::RandHelper(AvailableAttackNum - 1);
		switch (AvailableAttackNum)
		{
		case 0:CurrenAttackMode = EZombieAttackMode::LeftHand; break;
		case 1:CurrenAttackMode = EZombieAttackMode::RightHand; break;
		case 2:CurrenAttackMode = EZombieAttackMode::Hyper; break;
		default:CurrenAttackMode = EZombieAttackMode::LeftHand; break;
		}
		if (IsNetMode(NM_Standalone) || IsNetMode(NM_ListenServer))
		{
			OnRep_ZombieAttackMode();
			//actual hand hit point damage;
		}
		if (GetNetMode() == ENetMode::NM_DedicatedServer)
		{
			//line trace damage
		}
	}
}

void AINSZombie::PerfromLineTraceDamage()
{
	if (HasAuthority())
	{
		FRotator ViewRotation;
		FVector ViewPoint;
		GetActorEyesViewPoint(ViewPoint, ViewRotation);
		const float DamageRange = 100.f;
		FHitResult HitResult(ForceInit);
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(this);
		CollisionQueryParams.AddIgnoredComponent(GetCapsuleComponent());
		CollisionQueryParams.AddIgnoredComponent(GetMesh());
		GetWorld()->LineTraceSingleByChannel(HitResult, ViewPoint, ViewRotation.Vector()*DamageRange+ViewPoint, ECollisionChannel::ECC_Camera, CollisionQueryParams);
		if (HitResult.bBlockingHit)
		{
			ACharacter* const HitCharacter = Cast<ACharacter>(HitResult.GetActor());
			if (HitCharacter)
			{
				if (HitCharacter->GetClass()->IsChildOf(AINSZombie::StaticClass()))
				{
					return;
				}
				else if (HitCharacter->GetClass()->IsChildOf(AINSPlayerCharacter::StaticClass()))
				{
					FPointDamageEvent ZombieDamageEvent;
					ZombieDamageEvent.HitInfo = HitResult;
					ZombieDamageEvent.Damage = AttackDamage;
					HitCharacter->TakeDamage(AttackDamage, ZombieDamageEvent, ZombieController, this);
				}
			}
		}
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		DrawDebugLine(GetWorld(), ViewPoint, ViewRotation.Vector() * DamageRange + ViewPoint, FColor::Red, false, 5.f);
#endif
	}
}

void AINSZombie::OnRep_LastHitInfo()
{
	Super::OnRep_LastHitInfo();
	FName BoneName = LastHitInfo.HitBoneName;
}

void AINSZombie::FaceRotation(FRotator NewControlRotation, float DeltaTime /* = 0.f */)
{
	FRotator LaggedNewRotation(NewControlRotation.Pitch / 20.f, NewControlRotation.Yaw / 20.f, NewControlRotation.Roll / 20.f);
	Super::FaceRotation(LaggedNewRotation, DeltaTime);
}

void AINSZombie::OnRep_ZombieMoveMode()
{
	if (CachedZombieAnimInstance)
	{
		CachedZombieAnimInstance->SetZombieMoveMoe(CurrentZombieMoveMode);
	}
}

void AINSZombie::OnRep_ZombieAttackMode()
{
	if (CachedZombieAnimInstance)
	{
		UAnimMontage* SelectedAttackMontage = nullptr;
		switch (CurrenAttackMode)
		{
		case EZombieAttackMode::LeftHand:SelectedAttackMontage = ZombieAttackMontages.LeftHandAttackMontage; break;
		case EZombieAttackMode::RightHand:SelectedAttackMontage = ZombieAttackMontages.RighHandAttackMontage; break;
		case EZombieAttackMode::Hyper:SelectedAttackMontage = ZombieAttackMontages.HyperAttackMontage; break;
		default:SelectedAttackMontage = ZombieAttackMontages.LeftHandAttackMontage; break;
				
		}
		if (SelectedAttackMontage)
		{
			CachedZombieAnimInstance->Montage_Play(SelectedAttackMontage);
		}
		if (!SelectedAttackMontage)
		{
			UE_LOG(LogZombiePawn, Warning, TEXT("Zombie trying to play attack montage but no attack montage available"));
		}
	}
}

