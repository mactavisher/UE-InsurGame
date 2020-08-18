// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "INSGameModes/INSGameModeBase.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "INSComponents/INSHealthComponent.h"
#include "INSComponents/INSCharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "INSComponents/INSCharacterAudioComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundCue.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSItems\INSPickups\INSPickup_Weapon.h"
#include "INSComponents\INSWeaponMeshComponent.h"
#include "Kismet\GameplayStatics.h"
#include "Net/RepLayout.h"
#include "INSCharacter\INSCharacter.h"

DEFINE_LOG_CATEGORY(LogINSCharacter);

// Sets default values
AINSCharacter::AINSCharacter(const FObjectInitializer&ObjectInitializer) :Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	SetReplicateMovement(true);
	bIsDead = false;
	bIsAiming = false;
	bIsProne = false;
	bIsSprint = false;
	bIsCrouched = false;
	DefaultBaseEyeHeight = 90.f;
	bIsSuppressed = false;
	CharacterCurrentStance = ECharacterStance::STAND;
	NoiseEmmiterComp = ObjectInitializer.CreateDefaultSubobject<UPawnNoiseEmitterComponent>(this, TEXT("NoiseEmmiterComp"));
	CharacterHealthComp = ObjectInitializer.CreateDefaultSubobject<UINSHealthComponent>(this, TEXT("HealthComp"));
	INSCharacterMovementComp = CastChecked<UINSCharacterMovementComponent>(GetCharacterMovement());
	CharacterAudioComp = ObjectInitializer.CreateDefaultSubobject<UINSCharacterAudioComponent>(this, TEXT("AudioComp"));
	CharacterAudioComp->SetupAttachment(RootComponent);
	CachedTakeHitArray.SetNum(10);
#if WITH_EDITORONLY_DATA
	bShowDebugTrace = true;
#endif
}

// Called when the game starts or when spawned
void AINSCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority)
	{
		FScriptDelegate DieDelegate;
		DieDelegate.BindUFunction(this, TEXT("OnDeath"));
		CharacterHealthComp->OnCharacterShouldDie.Add(DieDelegate);
	}
}

void AINSCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
		DOREPLIFETIME_ACTIVE_OVERRIDE(AINSCharacter, LastHitInfo, !LastHitInfo.bIsDirtyData);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void AINSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CharacterAudioComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_HeadSocket"));
}


void AINSCharacter::ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	Super::ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
}

void AINSCharacter::HandleOnTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
}

void AINSCharacter::HandleOnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
}

void AINSCharacter::HandleOnTakeRadiusDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, FHitResult HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{

}

float AINSCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CharacterHealthComp->ReduceHealth(Damage, DamageCauser, EventInstigator);
	return Damage;
}

void AINSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSCharacter, bIsDead);
	DOREPLIFETIME_CONDITION(AINSCharacter, LastHitInfo, COND_Custom);
	DOREPLIFETIME(AINSCharacter, CharacterCurrentStance);
	DOREPLIFETIME(AINSCharacter, bIsAiming);
	DOREPLIFETIME(AINSCharacter, bIsProne);
	DOREPLIFETIME(AINSCharacter, CurrentWeapon);
	DOREPLIFETIME(AINSCharacter, bIsSprint);
}

void AINSCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	const float LandVelocity = GetVelocity().Size();
}

void AINSCharacter::CastBloodDecal(FVector HitLocation, FVector HitDir)
{
	UE_LOG(LogINSCharacter, Log, TEXT("calculate and cast blood decal"));
	FHitResult TraceHit;
	const float TraceRange = 100.f;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	const FVector TraceEnd = HitLocation + HitDir * TraceRange;
	GetWorld()->LineTraceSingleByChannel(TraceHit, HitLocation, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionQueryParams);
	if (TraceHit.bBlockingHit)
	{
		const UPrimitiveComponent* HitComp = TraceHit.Component.Get();
		if (HitComp && HitComp->Mobility == EComponentMobility::Static)
		{
			const int DecalNums = BloodSprayDecalMaterials.Num();
			if (DecalNums > 0)
			{
				const int RamdIndex = FMath::RandHelper(DecalNums);
				FRotator RandomDecalRotation = TraceHit.ImpactNormal.ToOrientationRotator();
				RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);
				UGameplayStatics::SpawnDecalAttached(BloodSprayDecalMaterials[RamdIndex], FVector(12.f, 12.f, 12.f), TraceHit.Component.Get(), TraceHit.BoneName, TraceHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition, 10.f);
			}
			UE_LOG(LogINSCharacter, Warning, TEXT("No blood decal to spawn,please config at least one in your blueprint setting!"));
		}
	}
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	if (bShowDebugTrace)
	{
		DrawDebugLine(GetWorld(), HitLocation, TraceEnd, FColor::Red, false, 2.f, 0, 5.f);
	}
#endif
}


void AINSCharacter::OnRep_Dead()
{
	GetINSCharacterMovement()->StopMovementImmediately();
}

void AINSCharacter::OnRep_LastHitInfo()
{
	if (LastHitInfo.bIsDirtyData)
	{
		UE_LOG(LogINSCharacter, Warning, TEXT("received Characters's:%s Dirty Hit Info!"));
		return;
	}
	UE_LOG(LogINSCharacter, Warning, TEXT("received Characters's:%s Hit Info,start handle take hit logic!"));
	//spawn blood hit Impact
	const FVector BloodSpawnLocation = LastHitInfo.RelHitLocation;
	const float ShotDirPitchDecompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirPitch);
	const float ShotDirYawDeCompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirYaw);
	const FRotator BloodSpawenRotation = FRotator(ShotDirPitchDecompressed, ShotDirYawDeCompressed, 0.f);
	const FTransform BloodSpawnTrans = FTransform(BloodSpawenRotation, BloodSpawnLocation, FVector::OneVector);
	const int32 BloodParticlesSize = BloodParticles.Num();
	const int32 randomIndex = FMath::RandHelper(BloodParticlesSize - 1);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodParticles[randomIndex], BloodSpawnTrans);
	UGameplayStatics::SpawnSoundAtLocation(this, BodyHitSound, LastHitInfo.RelHitLocation);
	const uint8 RandInt = FMath::RandHelper(10);
	if (RandInt % 3 == 0)
	{
		CastBloodDecal(BloodSpawnLocation, LastHitInfo.Momentum);
	}
	if (LastHitInfo.Damage > 0.f && !GetIsCharacterDead())
	{
		uint8 RamdonNum = FMath::RandHelper(7);
		if (RamdonNum % 3 == 0)
		{
			if (LastHitInfo.bIsTeamDamage)
			{
				CharacterAudioComp->SetVoiceType(EVoiceType::TEAMDAMAGE);
				CharacterAudioComp->PlayVoice();
			}
			else
			{
				CharacterAudioComp->SetVoiceType(EVoiceType::TAKEDAMAGE);
				CharacterAudioComp->PlayVoice();
			}
		}
	}
	CachedTakeHitArray.Add(LastHitInfo);
}

void AINSCharacter::OnRep_CurrentStance()
{

}

void AINSCharacter::OnRep_WantsToSwitchFireMode()
{

}

void AINSCharacter::OnRep_Sprint()
{
	if (!GetINSCharacterMovement())
	{
		return;
	}
	if (bIsSprint)
	{
		GetINSCharacterMovement()->StartSprint();
		OnStartSprint.Broadcast();
	}
	if (!bIsSprint)
	{
		GetINSCharacterMovement()->EndSprint();
		OnStopSprint.Broadcast();
	}
}

void AINSCharacter::OnRep_Aim()
{
	if (!GetINSCharacterMovement())
	{
		return;
	}
	if (bIsAiming)
	{
		GetINSCharacterMovement()->StartAim();
	}
	if (!bIsAiming)
	{
		GetINSCharacterMovement()->EndAim();
	}
}

void AINSCharacter::OnRep_Prone()
{
	if (!GetINSCharacterMovement())
	{
		return;
	}
	if (bIsProne)
	{
		GetINSCharacterMovement()->StartProne();
	}
	if (!bIsProne)
	{
		GetINSCharacterMovement()->EndProne();
	}
}


void AINSCharacter::OnRep_CurrentWeapon()
{

}

FORCEINLINE class UINSCharacterMovementComponent* AINSCharacter::GetINSCharacterMovement()
{
	return  Cast<UINSCharacterMovementComponent>(GetCharacterMovement());
}

void AINSCharacter::ReceiveHit(class AController*const InstigatorPlayer, class AActor* const DamageCauser, const FDamageEvent& DamageEvent, const FHitResult& Hit, float DamageTaken)
{
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		float DamageBeforeModify = DamageTaken;
		const FPointDamageEvent* const PointDamageEventPtr = (FPointDamageEvent*)&DamageEvent;
		if (GetLocalRole() == ROLE_Authority)
		{
			AINSGameModeBase* const GameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
			// modify any damage according to game rules and other settings
			if (GameMode)
			{
				GameMode->ModifyDamage(DamageTaken, InstigatorPlayer, this->GetController(), Hit.BoneName);
			}
			if (PointDamageEventPtr)
			{
				LastHitInfo.bIsDirtyData = true;
				LastHitInfo.bIsTeamDamage = GameMode->GetIsTeamDamage(InstigatorPlayer, GetController());
				LastHitInfo.originalDamage = DamageBeforeModify;
				LastHitInfo.Damage = GetIsCharacterDead() ? 0.f : DamageTaken;
				LastHitInfo.DamageCauser = DamageCauser;
				LastHitInfo.bVictimDead = GetIsCharacterDead();
				LastHitInfo.DamageInstigator = InstigatorPlayer;
				LastHitInfo.DamageType = DamageEvent.DamageTypeClass;
				LastHitInfo.Momentum = DamageCauser->GetVelocity();
				LastHitInfo.RelHitLocation = PointDamageEventPtr->HitInfo.ImpactPoint;
				const FVector ShotDir = DamageCauser->GetActorForwardVector();
				FRotator ShotRot = ShotDir.Rotation();
				LastHitInfo.ShotDirPitch = FRotator::CompressAxisToByte(ShotRot.Pitch);
				LastHitInfo.ShotDirYaw = FRotator::CompressAxisToByte(ShotRot.Yaw);
				LastHitInfo.bIsDirtyData = false;
				FTakeHitInfo ReplicatedHitInfo = LastHitInfo;
				if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
				{
					OnRep_LastHitInfo();
				}
				TakeDamage(DamageTaken, DamageEvent, InstigatorPlayer, DamageCauser);
			}
		}
	}
}

void AINSCharacter::HandleWeaponRealoadRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReloadWeapon();
	}
}

void AINSCharacter::HandleAimWeaponRequest()
{
	if (CurrentWeapon&&CurrentWeapon->GetWeaponCurrentState() != EWeaponState::RELOADIND)
	{
		CurrentWeapon->StartWeaponAim();
	}
}

void AINSCharacter::HandleStopAimWeaponRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopWeaponAim();
	}
}

void AINSCharacter::HandleFireRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->FireWeapon();
	}
}

void AINSCharacter::HandleStopFireRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopWeaponFire();
	}
}

void AINSCharacter::HandleEquipWeaponRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartEquipWeapon();
	}
}

void AINSCharacter::HandleSwitchFireModeRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartSwitchFireMode();
	}
}

void AINSCharacter::HandleMoveForwardRequest(float Value)
{
	if (Value != 0.0f)
	{
		// find out which way is forward
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// get forward vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AINSCharacter::HandleMoveRightRequest(float Value)
{
	if (Value != 0.0f)
	{
		// find out which way is Right
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// get Right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AINSCharacter::HandleCrouchRequest()
{
	if (bIsCrouched)
	{
		UnCrouch(true);
	}
	else
	{
		Crouch(true);
	}
}

void AINSCharacter::HandleStartSprintRequest()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsSprint = true;
		OnRep_Sprint();
	}
}

void AINSCharacter::HandleStopSprintRequest()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsSprint = false;
		OnRep_Sprint();
	}
}

void AINSCharacter::OnRep_IsCrouched()
{
	Super::OnRep_IsCrouched();
	if (bIsCrouched)
	{
		CharacterCurrentStance = ECharacterStance::CROUCH;
		if (GetINSCharacterMovement())
		{
			GetINSCharacterMovement()->StartCrouch();
		}
	}
}

void AINSCharacter::SpawnWeaponPickup()
{
	if (CurrentWeapon&&WeaponPickupClass)
	{
		class AINSPickup_Weapon* WeaponPickup = GetWorld()->SpawnActorDeferred<AINSPickup_Weapon>(WeaponPickupClass,
			CurrentWeapon->GetActorTransform(),
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		FVector WeaponLocation = CurrentWeapon->GetActorLocation();
		FTransform PickupSpawnTransform(CurrentWeapon->GetActorRotation(), WeaponLocation + FVector(0.f, 0.f, 50.f), FVector::OneVector);
		if (WeaponPickup)
		{
			WeaponPickup->SetActualWeaponClass(CurrentWeapon->GetClass());
			WeaponPickup->SetAmmoLeft(CurrentWeapon->AmmoLeft);
			WeaponPickup->SetCurrentClipAmmo(CurrentWeapon->CurrentClipAmmo);
			WeaponPickup->SetViualMesh(CurrentWeapon->WeaponMesh1PComp->SkeletalMesh);
			UGameplayStatics::FinishSpawningActor(WeaponPickup, PickupSpawnTransform);
		}
	}
	CurrentWeapon->Destroy(true);
}

void AINSCharacter::SetCurrentWeapon(class AINSWeaponBase* NewWeapon)
{
	this->CurrentWeapon = NewWeapon;
	NewWeapon->SetOwnerCharacter(this);
}


bool AINSCharacter::GetIsSuppressed() const
{
	return bIsSuppressed;
}

void AINSCharacter::BecomeViewTarget(APlayerController* PC)
{
	Super::BecomeViewTarget(PC);
}

void AINSCharacter::OnDeath()
{

}

void AINSCharacter::KilledBy(class AController* PlayerKilledMe, class AACtor* ActorKilledMe)
{

}

// Called every frame
void AINSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AINSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

