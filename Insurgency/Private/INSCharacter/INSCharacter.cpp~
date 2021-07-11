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
#include "Kismet/KismetStringLibrary.h"
#include "Sound/SoundCue.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSItems/INSPickups/INSPickup_Weapon.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/RepLayout.h"
#include "INSDamageTypes/INSDamageType_Falling.h"
#ifndef GEngine
#endif // !GEngine
#ifndef UWorld
#endif // !UWorld
#ifndef UCapsuleComponent
#include "Components/CapsuleComponent.h"
#endif // !UCapsuleComponent


DEFINE_LOG_CATEGORY(LogINSCharacter);

// Sets default values
AINSCharacter::AINSCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	bIsDead = false;
	bIsAiming = false;
	bIsProne = false;
	bIsSprint = false;
	bIsCrouched = false;
	bIsSuppressed = false;
	bDamageImmuneState = true;
	InitDamageImmuneTime = 5;
	DamageImmuneLeft = InitDamageImmuneTime;
	CharacterCurrentStance = ECharacterStance::STAND;
	FatalFallingSpeed = 2000.f;
	NoiseEmitterComp = ObjectInitializer.CreateDefaultSubobject<UPawnNoiseEmitterComponent>(
		this, TEXT("NoiseEmmiterComp"));
	CharacterHealthComp = ObjectInitializer.CreateDefaultSubobject<UINSHealthComponent>(this, TEXT("HealthComp"));
	if (CharacterHealthComp)
	{
		CharacterHealthComp->SetIsReplicated(true);
	}
	INSCharacterMovementComp = CastChecked<UINSCharacterMovementComponent>(GetCharacterMovement());
	CharacterAudioComp = ObjectInitializer.CreateDefaultSubobject<UINSCharacterAudioComponent>(this, TEXT("AudioComp"));
	CharacterAudioComp->SetupAttachment(RootComponent);
	CachedTakeHitArray.SetNum(10);
	GetMesh()->SetReceivesDecals(false);
#if WITH_EDITORONLY_DATA
	bShowDebugTrace = true;
#endif
}

void AINSCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		FScriptDelegate DieDelegate;
		DieDelegate.BindUFunction(this, TEXT("OnDeath"));
		CharacterHealthComp->OnCharacterShouldDie.Add(DieDelegate);
		GetWorldTimerManager().SetTimer(DamageImmuneTimer, this, &AINSCharacter::TickDamageImmune, 1.f, true, 0.f);
	}
	// we don't need sound comp in dedicated server,actually we can destroy it
	if (IsNetMode(NM_DedicatedServer))
	{
		if (CharacterAudioComp)
		{
			CharacterAudioComp->DestroyComponent(true);
			CharacterAudioComp = nullptr; //help GC
		}
	}
}

void AINSCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	DOREPLIFETIME_ACTIVE_OVERRIDE(AINSCharacter, LastHitInfo, !LastHitInfo.bIsDirtyData);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void AINSCharacter::GatherCurrentMovement()
{
	Super::GatherCurrentMovement();
}

void AINSCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
}


void AINSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CharacterAudioComp)
	{
		CharacterAudioComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
		                                      TEXT("Bip01_HeadSocket"));
		CharacterAudioComp->SetOwnerCharacter(this);
	}
}


void AINSCharacter::ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator,
                                        AActor* DamageCauser)
{
	Super::ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
}

void AINSCharacter::HandleOnTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy,
                                            FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName,
                                            FVector ShotFromDirection, const UDamageType* DamageType,
                                            AActor* DamageCauser)
{
}

void AINSCharacter::HandleOnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                          AController* InstigatedBy, AActor* DamageCauser)
{
}

void AINSCharacter::HandleOnTakeRadiusDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                             FVector Origin, FHitResult HitInfo, AController* InstigatedBy,
                                             AActor* DamageCauser)
{
}

float AINSCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                AActor* DamageCauser)
{
	if (HasAuthority())
	{
		if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		{
			float DamageAfterModify = 0.f;
			AINSGameModeBase* const GameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
			const FPointDamageEvent* const PointDamageEventPtr = (FPointDamageEvent*)&DamageEvent;
			// modify any damage according to game rules and other settings
			if (GameMode)
			{
				GameMode->ModifyDamage(DamageAfterModify
				                       , Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser)
				                       , EventInstigator, this->GetController()
				                       , DamageEvent
				                       , PointDamageEventPtr->HitInfo.BoneName);
			}
			const bool bFetalDamage = CharacterHealthComp->OnTakingDamage(DamageAfterModify, DamageCauser, EventInstigator);
			if (PointDamageEventPtr)
			{
				LastHitInfo.bIsTeamDamage = GameMode->GetIsTeamDamage(EventInstigator, GetController());
				LastHitInfo.Damage = GetIsDead() ? 0.f : DamageAfterModify;
				LastHitInfo.DamageCauser = DamageCauser;
				LastHitInfo.Victim = this;
				LastHitInfo.bVictimDead = bFetalDamage;
				LastHitInfo.DamageType = DamageEvent.DamageTypeClass;
				LastHitInfo.Momentum = DamageCauser->GetVelocity();
				LastHitInfo.bVictimAlreadyDead = !GetIsDead();
				LastHitInfo.InstigatorPawn = EventInstigator->GetPawn();
				LastHitInfo.RelHitLocation = PointDamageEventPtr->HitInfo.ImpactPoint;
				LastHitInfo.DamageType = DamageEvent.DamageTypeClass;
				const FVector ShotDir = DamageCauser->GetActorForwardVector();
				const FRotator ShotRot = ShotDir.Rotation();
				LastHitInfo.ShotDirPitch = FRotator::CompressAxisToByte(ShotRot.Pitch);
				LastHitInfo.ShotDirYaw = FRotator::CompressAxisToByte(ShotRot.Yaw);
				LastHitInfo.EnsureReplication();
				if (GameMode)
				{
					GameMode->PlayerScore(EventInstigator, GetController(), LastHitInfo);
				}
				if (IsNetMode(NM_Standalone) || IsNetMode(NM_ListenServer))
				{
					OnRep_LastHitInfo();
				}
				LastHitBy = EventInstigator;
			}
		}
		return Damage;
	}
	return 0.f;
}

bool AINSCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                     AActor* DamageCauser) const
{
	// ignore any kind of damage if we are in damage immune state
	if (bDamageImmuneState)
	{
		return false;
	}
	return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
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
	DOREPLIFETIME(AINSCharacter, DamageImmuneLeft);
}

void AINSCharacter::Landed(const FHitResult& Hit)
{
	UE_LOG(LogINSCharacter, Log, TEXT("Character %s just landed"), *GetName());
	if (HasAuthority())
	{
		Super::Landed(Hit);
		const AINSGameModeBase* const GM = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
		if (GM && GM->GetAllowFallingDamage())
		{
			const float LandVelocity = GetVelocity().Size();
			float Damage = 0.f;
			if (LandVelocity > 500.f)
			{
				Damage = FallingDamageCurve == nullptr ? 15.f : FallingDamageCurve->GetFloatValue(LandVelocity);
			}

			if (LandVelocity >= FatalFallingSpeed)
			{
				Damage = 90.f;
			}
			UE_LOG(LogINSCharacter
			       , Log
			       , TEXT("Character %s is taking land damage falling from high,initial damage taken:%f,landing speed %f")
			       , *GetName()
			       , *UKismetStringLibrary::Conv_FloatToString(Damage)
			       , LandVelocity);
			FPointDamageEvent FallingDamageEvent;
			FallingDamageEvent.DamageTypeClass = UINSDamageType_Falling::StaticClass();
			FallingDamageEvent.ShotDirection = Hit.ImpactNormal;
			FallingDamageEvent.HitInfo = Hit;
			TakeDamage(Damage, FallingDamageEvent, GetController(), this);
		}
	}
}

void AINSCharacter::CastBloodDecal(FVector HitLocation, FVector HitDir)
{
	UE_LOG(LogINSCharacter, Log, TEXT("calculate and cast blood decal"));
	FHitResult TraceHit;
	const float TraceRange = 100.f;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	const FVector TraceEnd = HitLocation + HitDir * TraceRange;
	GetWorld()->LineTraceSingleByChannel(TraceHit, HitLocation, TraceEnd, ECollisionChannel::ECC_Visibility,
	                                     CollisionQueryParams);
	if (TraceHit.bBlockingHit)
	{
		const UPrimitiveComponent* HitComp = TraceHit.Component.Get();
		if (HitComp && HitComp->Mobility == EComponentMobility::Static)
		{
			const int DecalNums = BloodSprayDecalMaterials.Num();
			if (DecalNums > 0)
			{
				const int RandomIdx = FMath::RandHelper(DecalNums);
				FRotator RandomDecalRotation = TraceHit.ImpactNormal.ToOrientationRotator();
				RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);
				UGameplayStatics::SpawnDecalAttached(BloodSprayDecalMaterials[RandomIdx]
				                                     , FVector(12.f, 12.f, 12.f)
				                                     , TraceHit.Component.Get()
				                                     , TraceHit.BoneName
				                                     , TraceHit.ImpactPoint
				                                     , RandomDecalRotation
				                                     , EAttachLocation::KeepWorldPosition
				                                     , 10.f);
			}
			UE_LOG(LogINSCharacter, Warning,
			       TEXT("No blood decal to spawn,please config at least one in your blueprint setting!"));
		}
	}
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	if (bShowDebugTrace)
	{
		DrawDebugLine(GetWorld(), HitLocation, TraceEnd, FColor::Red, false, 2.f, 0, 5.f);
	}
#endif
}


void AINSCharacter::OnCauseDamage(const FTakeHitInfo& HitInfo)
{
	if (LastHitInfo.Victim == LastHitInfo.InstigatorPawn)
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character %s is causing damge to himself"), *GetName());
		return;
	}
	// hook but do nothing by default
}

void AINSCharacter::SetLastHitStateInfo(class AActor* LastHitActor)
{
	LastHitState.LastHitActor = LastHitActor;
	LastHitState.CurrentHitStateLastTime = LastHitState.HitStateTime;
}

void AINSCharacter::OnRep_Dead()
{
	GetINSCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	if (!IsNetMode(NM_DedicatedServer) && GetCharacterAudioComp())
	{
		GetCharacterAudioComp()->OnDeath();
	}
}

void AINSCharacter::OnRep_LastHitInfo()
{
	if (!IsNetMode(NM_DedicatedServer))
	{
		UE_LOG(LogINSCharacter, Warning, TEXT("received Characters's:%s Hit Info,start handle take hit logic!"),
		       *GetName());
		SetLastHitStateInfo(LastHitInfo.DamageCauser);
		if (LastHitInfo.DamageType && !LastHitInfo.DamageType->IsChildOf(UINSDamageType_Falling::StaticClass()))
		{
			//spawn blood hit Impact
			const FVector BloodSpawnLocation = FVector(LastHitInfo.RelHitLocation);
			const float ShotDirPitchDecompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirPitch);
			const float ShotDirYawDeCompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirYaw);
			const FRotator BloodSpawenRotation = FRotator(ShotDirPitchDecompressed, ShotDirYawDeCompressed, 0.f);
			const FTransform BloodSpawnTrans = FTransform(BloodSpawenRotation, BloodSpawnLocation, FVector::OneVector);
			const int32 BloodParticlesSize = BloodParticles.Num();
			if (BloodParticlesSize > 1)
			{
				const int32 randomIndex = FMath::RandHelper(BloodParticlesSize - 1);
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodParticles[randomIndex], BloodSpawnTrans);
			}
			UGameplayStatics::SpawnSoundAtLocation(this, BodyHitSound, LastHitInfo.RelHitLocation);
			if (GetIsDead())
			{
				GetMesh()->AddImpulseToAllBodiesBelow(BloodSpawenRotation.Vector() * 2000.f, LastHitInfo.HitBoneName);
				GetMesh()->AddAngularImpulseInRadians(BloodSpawenRotation.Vector() * 2000.f, LastHitInfo.HitBoneName);
			}
			const uint8 RandInt = FMath::RandHelper(10);
			if (RandInt % 3 == 0)
			{
				CastBloodDecal(BloodSpawnLocation, LastHitInfo.Momentum);
			}
			if (LastHitInfo.Damage > 0.f && !GetIsDead())
			{
				const uint8 RamdonNum = FMath::RandHelper(7);
				if (RamdonNum % 3 == 0)
				{
					if (GetCharacterAudioComp())
					{
						GetCharacterAudioComp()->OnTakeDamage(LastHitInfo.bIsTeamDamage);
					}
				}
			}
		}
		AINSCharacter* const InstigatorCharacter = Cast<AINSCharacter>(LastHitInfo.InstigatorPawn);
		if (InstigatorCharacter)
		{
			InstigatorCharacter->OnCauseDamage(LastHitInfo);
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
	bIsAiming ? GetINSCharacterMovement()->StartAim() : GetINSCharacterMovement()->EndAim();
}

void AINSCharacter::OnRep_Prone()
{
	if (!GetINSCharacterMovement())
	{
		return;
	}
	bIsProne ? GetINSCharacterMovement()->StartProne() : GetINSCharacterMovement()->EndProne();
}


void AINSCharacter::OnRep_CurrentWeapon()
{
}

void AINSCharacter::OnRep_DamageImmuneTime()
{
	if (DamageImmuneLeft <= 0)
	{
		bDamageImmuneState = false;
	}
}

void AINSCharacter::TickDamageImmune()
{
	DamageImmuneLeft = FMath::Clamp<uint8>(DamageImmuneLeft - (uint8)1, 0, DamageImmuneLeft);
	if (DamageImmuneLeft <= 0)
	{
		bDamageImmuneState = false;
		GetWorldTimerManager().ClearTimer(DamageImmuneTimer);
	}
}

float AINSCharacter::GetCharacterCurrentHealth() const
{
	return GetCharacterHealthComp()->GetCurrentHealth();
}

void AINSCharacter::HandleWeaponReloadRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->HandleWeaponReloadRequest();
	}
}

void AINSCharacter::OnWeaponCollide(const FHitResult& Hit)
{
	if (Hit.bBlockingHit)
	{
		AActor* HitOtherActor = Hit.GetActor();
		if (HitOtherActor->GetClass()->IsChildOf(APawn::StaticClass()))
		{
			GetController()->SetIgnoreMoveInput(true);
		}
	}
	else
	{
		GetController()->SetIgnoreMoveInput(false);
	}
}

void AINSCharacter::TickHitStateTime(const float DeltaTime)
{
	if (!IsNetMode(NM_DedicatedServer))
	{
		if (LastHitState.CurrentHitStateLastTime > 0.f)
		{
			LastHitState.CurrentHitStateLastTime = FMath::Clamp<float>(LastHitState.CurrentHitStateLastTime - DeltaTime,
			                                                           0.f, LastHitState.CurrentHitStateLastTime);
		}
	}
}

void AINSCharacter::HandleAimWeaponRequest()
{
	if (HasAuthority())
	{
		if (CurrentWeapon && CurrentWeapon->GetWeaponCurrentState() != EWeaponState::RELOADIND)
		{
			CurrentWeapon->StartWeaponAim();
		}
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
		CurrentWeapon->StartWeaponFire();
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
	UE_LOG(LogINSCharacter, Log, TEXT("handle weapon switch fire mode request"));
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
	bIsCrouched ? UnCrouch(false) : Crouch(true);
}

void AINSCharacter::Crouch(bool bClientSimulation /* = false */)
{
	Super::Crouch(bClientSimulation);
}

void AINSCharacter::UnCrouch(bool bClientSimulation /* = false */)
{
	Super::UnCrouch(bClientSimulation);
}

void AINSCharacter::OnRep_IsCrouched()
{
	if (INSCharacterMovementComp)
	{
		UE_LOG(LogINSCharacter
		       , Log
		       , TEXT("Character % s crouch state replicated,is in crouch state:")
		       , *GetName()
		       , *UKismetStringLibrary::Conv_BoolToString(bIsCrouched));

		if (bIsCrouched)
		{
			CharacterCurrentStance = ECharacterStance::CROUCH;
			INSCharacterMovementComp->bWantsToCrouch = true;
			INSCharacterMovementComp->Crouch(true);
		}
		else
		{
			INSCharacterMovementComp->bWantsToCrouch = false;
			INSCharacterMovementComp->UnCrouch(true);
		}
		INSCharacterMovementComp->bNetworkUpdateReceived = true;
	}
}

void AINSCharacter::HandleStartSprintRequest()
{
	if (HasAuthority())
	{
		FVector Forward = GetActorForwardVector();
		FVector MoveDirection = GetINSCharacterMovement()->GetLastUpdateVelocity().GetSafeNormal();
		//Ignore vertical movement
		Forward.Z = 0.0f;
		MoveDirection.Z = 0.0f;
		float VelocityDot = FVector::DotProduct(Forward, MoveDirection);
		bIsSprint = VelocityDot > 0.8f && !GetCharacterMovement()->IsFalling();
		OnRep_Sprint();
	}
}

void AINSCharacter::HandleStopSprintRequest()
{
	if (HasAuthority())
	{
		bIsSprint = false;
		OnRep_Sprint();
	}
}

void AINSCharacter::HandleJumpRequest()
{
	if (HasAuthority())
	{
		if (CanJump() && !bPressedJump)
		{
			if (bIsSprint)
			{
				HandleStopSprintRequest();
			}
			Jump();
		}
	}
}

void AINSCharacter::HandleItemEquipRequest(const uint8 SlotIndex)
{
}

void AINSCharacter::SpawnWeaponPickup()
{
	if (CurrentWeapon && WeaponPickupClass)
	{
		class AINSPickup_Weapon* WeaponPickup = GetWorld()->SpawnActorDeferred<AINSPickup_Weapon>(WeaponPickupClass,
			CurrentWeapon->GetActorTransform(),
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		FVector WeaponLocation = CurrentWeapon->GetActorLocation();
		FTransform PickupSpawnTransform(CurrentWeapon->GetActorRotation(),
		                                WeaponLocation + FVector(0.f, 0.f, 50.f),
		                                FVector::OneVector);

		if (WeaponPickup)
		{
			WeaponPickup->SetActualWeaponClass(CurrentWeapon->GetClass());
			WeaponPickup->SetAmmoLeft(CurrentWeapon->AmmoLeft);
			WeaponPickup->SetLootableAmmo(CurrentWeapon->CurrentClipAmmo);
			UClass* const MeshClass = CurrentWeapon->WeaponMesh1PComp->SkeletalMesh->GetClass();
			WeaponPickup->SetViualMesh(NewObject<USkeletalMesh>(MeshClass));
			WeaponPickup->GetVisualMeshComp()->RegisterComponent();
			UGameplayStatics::FinishSpawningActor(WeaponPickup, PickupSpawnTransform);
		}
	}
	CurrentWeapon->Destroy(true);
}

void AINSCharacter::SetWeaponBasePoseType(const EWeaponBasePoseType NewType)
{
}

void AINSCharacter::SetCharacterAiming(bool NewAimState)
{
	if (HasAuthority())
	{
		this->bIsAiming = NewAimState;
		OnRep_Aim();
	}
}

void AINSCharacter::SetCurrentWeapon(class AINSWeaponBase* NewWeapon)
{
	this->CurrentWeapon = NewWeapon;
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_CurrentWeapon();
	}
	if (CurrentWeapon)
	{
		CurrentWeapon->StartEquipWeapon();
	}
}


bool AINSCharacter::GetIsSuppressed() const
{
	return bIsSuppressed;
}

void AINSCharacter::BecomeViewTarget(APlayerController* PC)
{
	Super::BecomeViewTarget(PC);
}

void AINSCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	Super::FellOutOfWorld(dmgType);
	OnDeath();
}

bool AINSCharacter::GetIsLowHealth() const
{
	return GetCharacterHealthComp()->CheckIsLowHealth();
}

void AINSCharacter::OnDeath()
{
	SetReplicates(false);
	TornOff();
}

bool AINSCharacter::GetIsCharacterMoving() const
{
	return GetINSCharacterMovement() && GetINSCharacterMovement()->GetLastUpdateVelocity().Size2D() > 0.f;
}

void AINSCharacter::OnLowHealth()
{
	UE_LOG(LogINSCharacter, Log, TEXT("Character %s received low health,Current health value is:%d"), *GetName(),
	       GetCurrentHealth());
}

float AINSCharacter::GetCurrentHealth() const
{
	return GetCharacterHealthComp()->GetCurrentHealth();
}

// Called every frame
void AINSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsCrouched)
	{
		CurrentEyeHeight = FMath::Clamp<float>(CurrentEyeHeight - 10.f, CrouchedEyeHeight, CurrentEyeHeight);
	}
	if (!bIsCrouched)
	{
		CurrentEyeHeight = FMath::Clamp<float>(CurrentEyeHeight + 10.f, CurrentEyeHeight, BaseEyeHeight);
	}
	TickHitStateTime(DeltaTime);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green,UKismetStringLibrary::Conv_BoolToString(bIsAiming));
}

void AINSCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

// Called to bind functionality to input
void AINSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
