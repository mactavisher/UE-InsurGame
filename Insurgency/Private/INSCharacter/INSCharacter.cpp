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
#include "INSGameModes/INSGameStateBase.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Kismet/KismetStringLibrary.h"
#ifndef GEngine
#include "Engine/Engine.h"
#endif // !GEngine
#ifndef UWorld
#include "Engine/World.h"
#endif // !UWorld
#ifndef UCapsuleComponent
#include "Components/CapsuleComponent.h"
#endif // !UCapsuleComponent


DEFINE_LOG_CATEGORY(LogINSCharacter);

// Sets default values
AINSCharacter::AINSCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	SetReplicateMovement(true);
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
	FatalFallingSpeed = 2016.f;
	bEnableFallingDamage = true;
	NoiseEmmiterComp = ObjectInitializer.CreateDefaultSubobject<UPawnNoiseEmitterComponent>(this, TEXT("NoiseEmmiterComp"));
	CharacterHealthComp = ObjectInitializer.CreateDefaultSubobject<UINSHealthComponent>(this, TEXT("HealthComp"));
	if (CharacterHealthComp)
	{
		CharacterHealthComp->SetIsReplicated(true);
	}
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
	if (HasAuthority())
	{
		FScriptDelegate DieDelegate;
		DieDelegate.BindUFunction(this, TEXT("OnDeath"));
		CharacterHealthComp->OnCharacterShouldDie.Add(DieDelegate);
		GetWorldTimerManager().SetTimer(DamageImmuneTimer, this, &AINSCharacter::TickDamageImmune, 1.f, true, 0.f);
	}
	// we don't need to attach the player sound comp in dedicated server,actually we can destroy it
	if (IsNetMode(NM_DedicatedServer))
	{
		CharacterAudioComp->DestroyComponent(true);
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

void AINSCharacter::GatherTakeHitInfo(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

}

void AINSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CharacterAudioComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_HeadSocket"));
	CharacterAudioComp->SetOwnerCharacter(this);
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
				GameMode->ModifyDamage(DamageAfterModify, Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser), EventInstigator, this->GetController(), DamageEvent, PointDamageEventPtr->HitInfo.BoneName);
			}
			if (PointDamageEventPtr)
			{
				FTakeHitInfo HitInfo;
				HitInfo.bIsTeamDamage = GameMode->GetIsTeamDamage(EventInstigator, GetController());
				HitInfo.Damage = GetIsCharacterDead() ? 0.f : DamageAfterModify;
				HitInfo.DamageCauser = DamageCauser;
				HitInfo.Victim = this;
				HitInfo.bVictimDead = GetIsCharacterDead();
				HitInfo.DamageType = DamageEvent.DamageTypeClass;
				HitInfo.Momentum = DamageCauser->GetVelocity();
				HitInfo.RelHitLocation = PointDamageEventPtr->HitInfo.ImpactPoint;
				const FVector ShotDir = DamageCauser->GetActorForwardVector();
				FRotator ShotRot = ShotDir.Rotation();
				HitInfo.ShotDirPitch = FRotator::CompressAxisToByte(ShotRot.Pitch);
				HitInfo.ShotDirYaw = FRotator::CompressAxisToByte(ShotRot.Yaw);
				HitInfo.EnsureReplication();
				LastHitInfo = HitInfo;
				if (IsNetMode(NM_Standalone) || IsNetMode(NM_ListenServer))
				{
					OnRep_LastHitInfo();
				}
			}
		}
		CharacterHealthComp->OnTakingDamage(LastHitInfo.Damage, DamageCauser, EventInstigator);
		if (GetIsCharacterDead())
		{
			AINSGameStateBase* CurrentGameState = GetWorld()->GetGameState<AINSGameStateBase>();
			AINSGameModeBase* CurrentGameMode = GetWorld()->GetAuthGameMode<AINSGameModeBase>();
			if (CurrentGameState && LastHitInfo.Damage > 0.f)
			{
				CurrentGameMode->ConfirmKill(EventInstigator, this->GetController(), FMath::CeilToInt(LastHitInfo.Damage), LastHitInfo.bIsTeamDamage);
			}
		}
		else
		{
			AINSGameStateBase* CurrentGameState = GetWorld()->GetGameState<AINSGameStateBase>();
			if (CurrentGameState && LastHitInfo.Damage > 0.f)
			{
				CurrentGameState->OnPlayerDamaged(EventInstigator, GetController(), LastHitInfo.Damage, LastHitInfo.bIsTeamDamage);
			}
		}
		return Damage;
	}
	return 0.f;
}

bool AINSCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)const
{
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
	if (HasAuthority())
	{
		Super::Landed(Hit);
		const float LandVelocity = GetVelocity().Size();
		float Damage = 0.f;
		if (bEnableFallingDamage)
		{
			if (LandVelocity > 500.f)
			{
				Damage = FallingDamageCurve == nullptr ? 15.f : FallingDamageCurve->GetFloatValue(LandVelocity);
			}

			if (LandVelocity >= FatalFallingSpeed)
			{
				Damage = 90.f;
			}
			FPointDamageEvent FallingDamageEvent;
			FallingDamageEvent.Damage = 70.f;
			FallingDamageEvent.ShotDirection = Hit.ImpactNormal;
			FallingDamageEvent.HitInfo = Hit;
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
				UGameplayStatics::SpawnDecalAttached(BloodSprayDecalMaterials[RamdIndex]
					, FVector(12.f, 12.f, 12.f)
					, TraceHit.Component.Get()
					, TraceHit.BoneName
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
		UE_LOG(LogINSCharacter, Warning, TEXT("received Characters's:%s Hit Info,start handle take hit logic!"), *GetName());
		//spawn blood hit Impact
		const FVector BloodSpawnLocation = FVector(LastHitInfo.RelHitLocation);
		const float ShotDirPitchDecompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirPitch);
		const float ShotDirYawDeCompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirYaw);
		const FRotator BloodSpawenRotation = FRotator(ShotDirPitchDecompressed, ShotDirYawDeCompressed, 0.f);
		const FTransform BloodSpawnTrans = FTransform(BloodSpawenRotation, BloodSpawnLocation, FVector::OneVector);
		const int32 BloodParticlesSize = BloodParticles.Num();
		const int32 randomIndex = FMath::RandHelper(BloodParticlesSize - 1);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodParticles[randomIndex], BloodSpawnTrans);
		UGameplayStatics::SpawnSoundAtLocation(this, BodyHitSound, LastHitInfo.RelHitLocation);
		if (GetIsCharacterDead())
		{
			GetMesh()->AddImpulseToAllBodiesBelow(BloodSpawenRotation.Vector() * 800.f, LastHitInfo.HitBoneName);
			GetMesh()->AddAngularImpulseInRadians(BloodSpawenRotation.Vector() * 800.f, LastHitInfo.HitBoneName);
		}
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
					if (GetCharacterAudioComp())
					{
						GetCharacterAudioComp()->OnTakeDamage(true);
					}
				}
				else
				{
					if (GetCharacterAudioComp())
					{
						GetCharacterAudioComp()->OnTakeDamage(false);
					}
				}
			}
		}
	}
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	if (LastHitInfo.Victim == this)
	{
		FString DebugMessage;
		DebugMessage.Append("you are taking damage, damage token: ").Append(FString::FromInt(LastHitInfo.Damage));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, DebugMessage);
	}
#endif
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

void AINSCharacter::HandleWeaponRealoadRequest()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReloadWeapon();
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
	Super::OnRep_IsCrouched();
	if (bIsCrouched)
	{
		CharacterCurrentStance = ECharacterStance::CROUCH;
		if (GetINSCharacterMovement())
		{
			GetINSCharacterMovement()->StartCrouch();
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Crouch replicated"));
}

void AINSCharacter::HandleStartSprintRequest()
{
	if (HasAuthority())
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Handle sprint request"));
			if (!GetCharacterMovement()->GetLastInputVector().IsNearlyZero())
			{
				FMatrix RotMatrix = FRotationMatrix(GetActorForwardVector().ToOrientationRotator());
				FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
				FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
				FVector NormalizedVel = GetCharacterMovement()->GetLastInputVector().GetSafeNormal2D();

				// get a cos(alpha) of forward vector vs velocity
				float ForwardCosAngle = FVector::DotProduct(ForwardVector, NormalizedVel);
				// now get the alpha and convert to degree
				float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

				// depending on where right vector is, flip it
				float RightCosAngle = FVector::DotProduct(RightVector, NormalizedVel);
				if (RightCosAngle < 0)
				{
					ForwardDeltaDegree *= -1;
				}
				UE_LOG(LogINSCharacter, Log, TEXT("sprint requset calculated delta:%f"), ForwardDeltaDegree);
				bIsSprint = FMath::IsNearlyEqual(ForwardDeltaDegree,0,1);
				OnRep_Sprint();
			}
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
			WeaponPickup->SetCurrentClipAmmo(CurrentWeapon->CurrentClipAmmo);
			UClass* const MeshClass = CurrentWeapon->WeaponMesh1PComp->SkeletalMesh->GetClass();
			WeaponPickup->SetViualMesh(NewObject<USkeletalMesh>(MeshClass));
			WeaponPickup->GetVisualMeshComp()->RegisterComponent();
			UGameplayStatics::FinishSpawningActor(WeaponPickup, PickupSpawnTransform);
		}
	}
	CurrentWeapon->Destroy(true);
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
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwnerCharacter(this);
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

bool AINSCharacter::GetIsLowHealth() const
{
	return GetCharacterHealthComp()->CheckIsLowHealth();
}

void AINSCharacter::OnDeath()
{

}

bool AINSCharacter::GetIsCharacterMoving() const
{
	return GetINSCharacterMovement() != nullptr && GetINSCharacterMovement()->GetLastUpdateVelocity().Size2D() > 0.f;
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

