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
#include "Kismet/GameplayStatics.h"
#include "Net/RepLayout.h"
#include "INSDamageTypes/INSDamageType_Falling.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerState.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "INSAssets/INSStaticAnimData.h"
#include "INSCore/INSGameInstance.h"
#ifndef GEngine
#endif // !GEngine
#ifndef UWorld
#endif // !UWorld
#ifndef UCapsuleComponent
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#endif // !UCapsuleComponent


DEFINE_LOG_CATEGORY(LogINSCharacter);

// Sets default values
AINSCharacter::AINSCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharacterMovementComponent>(CharacterMovementComponentName))
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
	NoiseEmitterComp = ObjectInitializer.CreateDefaultSubobject<UPawnNoiseEmitterComponent>(this, TEXT("NoiseEmmiterComp"));
	CharacterHealthComp = ObjectInitializer.CreateDefaultSubobject<UINSHealthComponent>(this, TEXT("HealthComp"));
	INSCharacterMovementComp = CastChecked<UINSCharacterMovementComponent>(GetCharacterMovement());
	CharacterAudioComp = ObjectInitializer.CreateDefaultSubobject<UINSCharacterAudioComponent>(this, TEXT("AudioComp"));
	PhysicalAnimationComponent = ObjectInitializer.CreateDefaultSubobject<UPhysicalAnimationComponent>(this, TEXT("PhysicalAnimationComponent"));
	CharacterAudioComp->SetupAttachment(RootComponent);
	CachedTakeHitArray.SetNum(10);
	GetMesh()->SetReceivesDecals(false);
	DeathTime = 0.f;
	WeaponPickupClass = AINSPickup_Weapon::StaticClass();
#if WITH_EDITORONLY_DATA
	bShowDebugTrace = true;
#endif
	if (CharacterHealthComp)
	{
		CharacterHealthComp->SetIsReplicated(true);
	}
	DeathTime = 0.f;
	CurrentWeapon = nullptr;
	CurrentAnimPtr = nullptr;
}

void AINSCharacter::BeginPlay()
{
	Super::BeginPlay();
	// we don't need sound comp in dedicated server,so destroy it
	if (IsNetMode(NM_DedicatedServer))
	{
		if (CharacterAudioComp)
		{
			CharacterAudioComp->DestroyComponent(true);
			CharacterAudioComp = nullptr; //help GC
		}
	}
	if (HasAuthority())
	{
		if (bDamageImmuneState)
		{
			GetWorldTimerManager().SetTimer(DamageImmuneTimer, this, &AINSCharacter::TickDamageImmune, 1.f, true, 0.f);
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
}


void AINSCharacter::ApplyDamageMomentum(float DamageTaken, const FDamageEvent& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	Super::ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
}

void AINSCharacter::HandleOnTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType,
                                            AActor* DamageCauser)
{
}

void AINSCharacter::HandleOnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
}

void AINSCharacter::HandleOnTakeRadiusDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, FHitResult HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
}

bool AINSCharacter::ShouldTakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	// ignore any kind of damage if we are in damage immune state
	if (bDamageImmuneState)
	{
		return false;
	}
	return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

float AINSCharacter::TakeDamage(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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
				DamageAfterModify = GameMode->ModifyDamage(Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser), EventInstigator, this->GetController(), DamageEvent);
			}
			if (PointDamageEventPtr)
			{
				CharacterHealthComp->OnTakingDamage(DamageAfterModify, DamageCauser, EventInstigator);
				LastHitInfo.bValidShot = true;
				LastHitInfo.Damage = GetIsDead() ? 0.f : DamageAfterModify;
				LastHitInfo.DamageCauser = DamageCauser;
				LastHitInfo.Victim = this;
				LastHitInfo.bVictimDead = GetCurrentHealth() - DamageAfterModify <= 0.f;
				LastHitInfo.DamageType = DamageEvent.DamageTypeClass;
				LastHitInfo.Momentum = DamageCauser->GetVelocity();
				LastHitInfo.bVictimAlreadyDead = !GetIsDead();
				LastHitInfo.InstigatorPawn = EventInstigator->GetPawn();
				LastHitInfo.RelHitLocation = PointDamageEventPtr->HitInfo.ImpactPoint;
				LastHitInfo.DamageType = DamageEvent.DamageTypeClass;
				const FVector ShotDir = DamageCauser->GetActorForwardVector();
				const FRotator ShotRot = PointDamageEventPtr->HitInfo.ImpactNormal.Rotation();
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
				LastHitInfo.bIsTeamDamage = GameMode->GetIsTeamDamage(EventInstigator, GetController());
				LastHitBy = EventInstigator;
			}
		}
		return Damage;
	}
	return 0.f;
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
			const float LandZVelocity = FMath::Abs(GetVelocity().Z);
			float Damage = 0.f;
			if (LandZVelocity > 800.f)
			{
				Damage = FallingDamageCurve == nullptr ? 15.f : FallingDamageCurve->GetFloatValue(LandZVelocity);
			}
			if (LandZVelocity >= FatalFallingSpeed)
			{
				Damage = 90.f;
			}
			UE_LOG(LogINSCharacter, Log, TEXT("Character %s is taking land damage falling from high,initial damage taken:%f,landing speed %f"), *GetName(), *FString::SanitizeFloat(Damage), LandZVelocity);
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
	GetWorld()->LineTraceSingleByChannel(TraceHit, HitLocation, TraceEnd, ECC_Visibility, CollisionQueryParams);
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
				UGameplayStatics::SpawnDecalAttached(BloodSprayDecalMaterials[RandomIdx], FVector(12.f, 12.f, 12.f), TraceHit.Component.Get(), TraceHit.BoneName
				                                     , TraceHit.ImpactPoint
				                                     , RandomDecalRotation
				                                     , EAttachLocation::KeepWorldPosition
				                                     , 10.f);
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


void AINSCharacter::TossCurrentWeapon()
{
	if (CurrentWeapon && WeaponPickupClass)
	{
		class AINSPickup_Weapon* WeaponPickup = GetWorld()->SpawnActorDeferred<AINSPickup_Weapon>(WeaponPickupClass, CurrentWeapon->GetActorTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		const FVector WeaponLocation = CurrentWeapon->GetActorLocation();
		const FTransform PickupSpawnTransform(CurrentWeapon->GetActorRotation(), WeaponLocation + FVector(0.f, 0.f, 50.f), FVector::OneVector);
		if (WeaponPickup)
		{
			WeaponPickup->SetActualWeaponClass(CurrentWeapon->GetClass());
			WeaponPickup->SetAmmoLeft(CurrentWeapon->GetAmmoLeft());
			WeaponPickup->SetLootableAmmo(CurrentWeapon->GetCurrentClipAmmo());
			const FSoftObjectPath AssetsRef(CurrentWeapon->GetWeaponStaticMesh());
			UStaticMesh* WeaponMesh = LoadObject<UStaticMesh>(this, *AssetsRef.ToString());
			UGameplayStatics::FinishSpawningActor(WeaponPickup, PickupSpawnTransform);
			FRepPickupInfo RepPickupInfo;
			RepPickupInfo.VisualAssetPath = AssetsRef.ToString();
			WeaponPickup->SetRepPickupInfo(RepPickupInfo);
			WeaponPickup->GetVisualMeshComp()->SetStaticMesh(WeaponMesh);
			CurrentWeapon->Destroy(true);
#if WITH_EDITOR
			WeaponPickup->GetSimpleCollisionComp()->SetHiddenInGame(false);
#endif
			WeaponPickup->GetSimpleCollisionComp()->SetSimulatePhysics(true);
			WeaponPickup->GetSimpleCollisionComp()->AddImpulseAtLocation(WeaponPickup->GetActorForwardVector() * 500.f + FVector(0.f, 0.f, 200.f), WeaponPickup->GetActorLocation(), NAME_None);
			WeaponPickup->GetSimpleCollisionComp()->AddAngularImpulseInRadians(FVector(100.f, 300.f, 20.f));
		}
	}
}

bool AINSCharacter::CheckCharacterIsReady()
{
	return GetMesh() && GetMesh()->HasBegunPlay() && IsValid(GetMesh()->SkeletalMesh) && IsValid(GetMesh()->SkeletalMesh->GetSkeleton());
}

void AINSCharacter::ReceiveInventoryInitialized()
{
}

void AINSCharacter::ReceiveClipAmmoEmpty()
{
}

void AINSCharacter::ReceiveSetupWeaponAttachment()
{
}


void AINSCharacter::CreateAndEquipItem(int32 ItemId, const uint8 InventorySlotIndex)
{
	UINSGameInstance* CurGameInstance = GetWorld()->GetGameInstance<UINSGameInstance>();
	if (CurGameInstance)
	{
		UINSItemManager* ItemManager = CurGameInstance->GetItemManager();
		if (ItemManager)
		{
			AINSWeaponBase* WeaponItemInstance = ItemManager->CreateWeaponItemInstance(ItemId, GetActorTransform(), GetController(), this, InventorySlotIndex);
			if (WeaponItemInstance)
			{
				SetCurrentWeapon(WeaponItemInstance);
			}
		}
	}
}


void AINSCharacter::OnCauseDamage(const FTakeHitInfo& HitInfo)
{
	if (LastHitInfo.Victim == LastHitInfo.InstigatorPawn)
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character %s is causing damge to himself"), *GetName());
	}
	// hook but do nothing by default
}

void AINSCharacter::SetLastHitStateInfo(class AActor* LastHitActor)
{
	LastHitState.LastHitActor = LastHitActor;
	LastHitState.CurrentHitStateLastTime = LastHitState.HitStateTime;
}

void AINSCharacter::SetCurrentAnimData(UINSStaticAnimData* AnimData)
{
	CurrentAnimPtr = AnimData;
}

bool AINSCharacter::GetIsABot()
{
	return GetPlayerState() && GetPlayerState()->IsABot();
}

bool AINSCharacter::GetCharacterIsAlreadyDead()
{
	return bIsDead && GetWorld()->DeltaTimeSeconds >= DeathTime;
}

float AINSCharacter::CheckDistance(const FVector OtherLocation)
{
	return (GetActorLocation() - OtherLocation).Size();
}

void AINSCharacter::SetupPendingWeaponEquipEvent(const int32 ItemId, const uint8 ItemSlotIdx)
{
	if (!PendingWeaponEquipEvent.bIsEventActive)
	{
		PendingWeaponEquipEvent.bIsEventActive = true;
		PendingWeaponEquipEvent.ItemId = ItemId;
		PendingWeaponEquipEvent.EventCreateTime = GetWorld()->GetTimeSeconds();
		PendingWeaponEquipEvent.WeaponSlotIndex = ItemSlotIdx;
	}
}

FPendingWeaponEquipEvent& AINSCharacter::GetPendingEquipEvent()
{
	return PendingWeaponEquipEvent;
}

void AINSCharacter::UpdateAnimationData(class AINSItems* InItemRef)
{
}

void AINSCharacter::OnRep_LastHitInfo()
{
	if (!IsNetMode(NM_DedicatedServer))
	{
		UE_LOG(LogINSCharacter, Warning, TEXT("received Characters's:%s Hit Info,start handle take hit logic!"), *GetName());
		SetLastHitStateInfo(LastHitInfo.DamageCauser);
		if (LastHitInfo.DamageType && !LastHitInfo.DamageType->IsChildOf(UINSDamageType_Falling::StaticClass()))
		{
			//spawn blood hit Impact
			const FVector BloodSpawnLocation = FVector(LastHitInfo.RelHitLocation);
			const float ShotDirPitchDecompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirPitch);
			const float ShotDirYawDeCompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirYaw);
			const FRotator BloodSpawnRotation = FRotator(ShotDirPitchDecompressed, ShotDirYawDeCompressed, 0.f);
			//const FTransform BloodSpawnTrans = FTransform(BloodSpawnRotation, BloodSpawnLocation + GetMesh()->GetComponentLocation(), FVector::OneVector);
			const int32 BloodParticlesSize = BloodParticles.Num();
			if (BloodParticlesSize > 0)
			{
				const int32 RandomIndex = FMath::RandHelper(BloodParticlesSize - 1);
				//UGameplayStatics::SpawnEmitterAttached(GetWorld(), BloodParticles[RandomIndex], BloodSpawnTrans);
				UGameplayStatics::SpawnEmitterAttached(BloodParticles[RandomIndex], GetMesh(), NAME_None, BloodSpawnLocation, BloodSpawnRotation, FVector::OneVector, EAttachLocation::KeepWorldPosition);
			}
			UGameplayStatics::SpawnSoundAtLocation(this, BodyHitSound, LastHitInfo.RelHitLocation);
			const uint8 RandInt = FMath::RandHelper(10);
			if (RandInt % 3 == 0)
			{
				CastBloodDecal(BloodSpawnLocation, LastHitInfo.Momentum);
			}
			if (LastHitInfo.Damage > 0.f && !GetIsDead())
			{
				const uint8 RandomNum = FMath::RandHelper(7);
				if (RandomNum % 3 == 0)
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
	ApplyPhysicAnimation();
	CachedTakeHitArray.Add(LastHitInfo);
	DeathTime = GetWorld()->TimeSeconds;
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
	else
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
	DamageImmuneLeft = FMath::Clamp<uint8>(DamageImmuneLeft - static_cast<uint8>(1), 0, DamageImmuneLeft);
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
		if (HasAuthority())
		{
			CurrentWeapon->HandleWeaponReloadRequest();
		}
		else
		{
			CurrentWeapon->ServerHandleWeaponReloadRequest();
		}
	}
}

void AINSCharacter::HandleItemEquipRequest(int32 ItemId, uint8 ItemSlotIndex)
{
}

void AINSCharacter::OnWeaponCollide(const FHitResult& Hit)
{
	if (Hit.bBlockingHit)
	{
		AActor* HitOtherActor = Hit.GetActor();
		if (HitOtherActor && HitOtherActor->GetClass()->IsChildOf(APawn::StaticClass()))
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
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	if (LastHitState.CurrentHitStateLastTime > 0.f)
	{
		LastHitState.CurrentHitStateLastTime = FMath::Clamp<float>(LastHitState.CurrentHitStateLastTime - DeltaTime, 0.f, LastHitState.CurrentHitStateLastTime);
	}
}

void AINSCharacter::HandleAimWeaponRequest()
{
	if (!HasAuthority())
	{
		return;
	}
	if (CurrentWeapon && CurrentWeapon->GetWeaponCurrentState() != EWeaponState::RELOADIND)
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
	if (PendingWeaponEquipEvent.bIsEventActive)
	{
		return;
	}
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

void AINSCharacter::HandleSingleAmmoInsertRequest()
{
	if (CurrentWeapon)
	{
		if (HasAuthority())
		{
			CurrentWeapon->InsertSingleAmmo();
		}
		else if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			CurrentWeapon->ServerInsertSingleAmmo();
		}
	}
}

void AINSCharacter::HandleFinishReloadingRequest()
{
	if (CurrentWeapon)
	{
		if (HasAuthority())
		{
			CurrentWeapon->FinishReloadWeapon();
		}
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			CurrentWeapon->ServerFinishReloadWeapon();
		}
	}
}

void AINSCharacter::HandleFinishUnEquipWeaponRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->SetWeaponState(EWeaponState::UNEQUIPED);
	}
}

void AINSCharacter::HandleItemFinishEquipRequest()
{
	if (CurrentWeapon)
	{
		if (HasAuthority())
		{
			CurrentWeapon->SetWeaponState(EWeaponState::IDLE);
		}
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			CurrentWeapon->ServerSetWeaponState(EWeaponState::IDLE);
		}
	}
	if (GetLocalRole() >= ROLE_AutonomousProxy && !IsNetMode(NM_DedicatedServer))
	{
		PendingWeaponEquipEvent.ResetEvent();
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

void AINSCharacter::Die()
{
	if (!HasAuthority() && GetIsDead())
	{
		return;
	}
	bIsDead = true;
	if (CurrentWeapon)
	{
		TossCurrentWeapon();
	}
	SetReplicateMovement(false);
	TearOff();
	OnRep_Dead();
}

void AINSCharacter::ServerCreateAndEquipItem_Implementation(int32 ItemId, const uint8 InventorySlotIndex)
{
	CreateAndEquipItem(ItemId, InventorySlotIndex);
}

bool AINSCharacter::ServerCreateAndEquipItem_Validate(int32 ItemId, const uint8 InventorySlotIndex)
{
	return true;
}

void AINSCharacter::OnRep_Dead()
{
	if (bIsDead)
	{
		if (IsNetMode(NM_DedicatedServer))
		{
			if (GetMesh())
			{
				GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
			}
		}
		GetCharacterHealthComp()->DisableComponentTick();
		GetINSCharacterMovement()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
		if (!IsNetMode(NM_DedicatedServer) && GetCharacterAudioComp())
		{
			GetCharacterAudioComp()->OnDeath();
		}
		const uint8 BloodFlowNum = BloodFlowParticles.Num();
		if (BloodFlowNum > 0)
		{
			const float ShotDirPitchDecompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirPitch);
			const float ShotDirYawDeCompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirYaw);
			const FRotator BloodSpawnRotation = FRotator(ShotDirPitchDecompressed, ShotDirYawDeCompressed, 0.f);
			const uint8 RandomNum = FMath::RandHelper(BloodFlowNum - 1);
			UGameplayStatics::SpawnEmitterAttached(BloodFlowParticles[RandomNum], GetMesh(), NAME_None, LastHitInfo.RelHitLocation, BloodSpawnRotation, FVector::OneVector * 1.5f, EAttachLocation::KeepWorldPosition);
		}
		SetLifeSpan(10.f);
	}
}

void AINSCharacter::OnRep_IsCrouched()
{
	if (INSCharacterMovementComp)
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character % s crouch state replicated,is in crouch state:"), *GetName(), *UKismetStringLibrary::Conv_BoolToString(bIsCrouched));

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
		if (FVector::DotProduct(Forward, MoveDirection) > 0.8f && !GetCharacterMovement()->IsFalling())
		{
			//if we are currently Aiming ,stop it
			if (bIsAiming)
			{
				HandleStopAimWeaponRequest();
			}
			bIsSprint = true;
		}
		OnRep_Sprint();
	}
}

void AINSCharacter::HandleStopSprintRequest()
{
	if (!HasAuthority())
	{
		return;
	}
	bIsSprint = false;
	OnRep_Sprint();
}

void AINSCharacter::HandleJumpRequest()
{
	if (HasAuthority())
	{
		if (bIsSprint)
		{
			HandleStopSprintRequest();
		}
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
	}
	if (CanJump() && !bPressedJump)
	{
		Jump();
	}
}


void AINSCharacter::HandleItemFinishUnEquipRequest()
{
	if (HasAuthority())
	{
		FinishUnEquipItem();
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerFinishUnEquipItem();
	}
}

void AINSCharacter::UnEquipItem()
{
	if (PendingWeaponEquipEvent.bIsEventActive)
	{
		return;
	}
	if (CurrentWeapon)
	{
		CurrentWeapon->StartUnEquipWeapon();
	}
}

void AINSCharacter::FinishUnEquipItem()
{
	CurrentWeapon->Destroy();
	CurrentWeapon = nullptr;
}

void AINSCharacter::ServerFinishUnEquipItem_Implementation()
{
	FinishUnEquipItem();
}

bool AINSCharacter::ServerFinishUnEquipItem_Validate()
{
	return true;
}

void AINSCharacter::ServerUnEquipItem_Implementation()
{
	UnEquipItem();
}

bool AINSCharacter::ServerUnEquipItem_Validate()
{
	return true;
}


void AINSCharacter::SpawnWeaponPickup()
{
}

void AINSCharacter::SetAimHandsXLocation(const float Value)
{
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
	if (HasAuthority())
	{
		OnRep_CurrentWeapon();
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
	Die();
}

bool AINSCharacter::GetIsLowHealth() const
{
	return GetCharacterHealthComp()->CheckIsLowHealth();
}


bool AINSCharacter::GetIsCharacterMoving() const
{
	return GetINSCharacterMovement() && GetINSCharacterMovement()->GetLastUpdateVelocity().Size2D() > 0.f;
}

void AINSCharacter::OnLowHealth()
{
	UE_LOG(LogINSCharacter, Log, TEXT("Character %s received low health,Current health value is:%d"), *GetName(), GetCurrentHealth());
}

float AINSCharacter::GetCurrentHealth() const
{
	return GetCharacterHealthComp()->GetCurrentHealth();
}

void AINSCharacter::ApplyPhysicAnimation()
{
}

// Called every frame
void AINSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// if (bIsCrouched)
	// {
	// 	CurrentEyeHeight = FMath::Clamp<float>(CurrentEyeHeight - 10.f, CrouchedEyeHeight, CurrentEyeHeight);
	// }
	// if (!bIsCrouched)
	// {
	// 	CurrentEyeHeight = FMath::Clamp<float>(CurrentEyeHeight + 10.f, CurrentEyeHeight, BaseEyeHeight);
	// }
	TickHitStateTime(DeltaTime);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green,UKismetStringLibrary::Conv_BoolToString(bIsAiming))
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
