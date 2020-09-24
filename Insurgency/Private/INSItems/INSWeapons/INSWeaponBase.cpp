// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "INSProjectiles/INSProjectile.h"
#include "INSCharacter/INSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "INSAnimation/INSCharacterAimInstance.h"
#include "Camera/CameraShake.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundCue.h"
#include "INSEffects/INSProjectileShell.h"
#include "TimerManager.h"
#include "INSItems/INSItems.h"
#include "Particles/ParticleSystemComponent.h"
#include "INSAssets/INSWeaponAssets.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "INSCharacter/INSPlayerCameraManager.h"
#include "INSAnimation/INSWeaponAnimInstance.h"
#include "INSHud/INSHUDBase.h"
#include "INSComponents/INSCharacterAudioComponent.h"

DEFINE_LOG_CATEGORY(LogINSWeapon);

AINSWeaponBase::AINSWeaponBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	//ItemType = EItemType::WEAPON;
	AvailableFireModes.Add(EWeaponFireMode::FULLAUTO);
	AvailableFireModes.Add(EWeaponFireMode::SEMI);
	CurrentWeaponFireMode = AvailableFireModes[0];
	CurrentClipAmmo = WeaponConfigData.AmmoPerClip;
	AmmoLeft = WeaponConfigData.MaxAmmo;
	CurrentWeaponState = EWeaponState::NONE;
	LastFireTime = 0.f;
	RepWeaponFireCount = 0;
	bisAiming = false;
	bInfinitAmmo = false;
	bForeGripEquipt = false;
	SemiAutoCurrentRoundCount = 0;
	AimTime = 0.2f;
	RecoilVerticallyFactor = -3.f;
	RecoilHorizontallyFactor = 6.f;
	NetUpdateFrequency = 10.f;
	BaseHandsIk = FVector::ZeroVector;
	//AdjustADSHandsIK = FVector(-10.f, -3.8f, 5.6f);
	AdjustADSHandsIK = FVector(ForceInit);
	bDryReload = false;
	SetReplicateMovement(false);
	WeaponMesh1PComp = ObjectInitializer.CreateDefaultSubobject<UINSWeaponMeshComponent>(this, TEXT("WeaponMesh1PComp"));
	WeaponMesh3PComp = ObjectInitializer.CreateDefaultSubobject<UINSWeaponMeshComponent>(this, TEXT("WeaponMesh3PComp"));
	WeaponParticleComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticelSystemComp"));
	WeaponMesh1PComp->AlwaysLoadOnClient = true;
	WeaponMesh1PComp->AlwaysLoadOnServer = true;
	WeaponMesh3PComp->AlwaysLoadOnClient = true;
	WeaponMesh3PComp->AlwaysLoadOnServer = true;
	RootComponent = WeaponMesh1PComp;
	WeaponMesh3PComp->SetupAttachment(RootComponent);
	WeaponParticleComp->SetupAttachment(RootComponent);
	WeaponMesh1PComp->SetHiddenInGame(true);
	WeaponMesh3PComp->SetHiddenInGame(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	bWantsToEquip = false;
	bEnableAutoReload = true;
#if WITH_EDITORONLY_DATA
	bShowDebugTrace = false;
#endif
}

void AINSWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentWeaponState == EWeaponState::IDLE)
	{
		OnWeaponEnterIdle.Broadcast();
	}
	if (OwnerCharacter&&OwnerCharacter->IsLocallyControlled())
	{
		UpdateWeaponSpread(DeltaTime);
		if (CurrentWeaponState == EWeaponState::FIRING)
		{
			UpdateRecoilHorizontally(DeltaTime, GetRecoilHorizontallyFactor());
			UpdateRecoilVertically(DeltaTime, GetRecoilVerticallyFactor());
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 0.1, FColor::Green, GetWeaponReadableCurrentState());
}

void AINSWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AINSWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	SetupWeaponMeshRenderings();
}

void AINSWeaponBase::FireWeapon()
{
	if (!CheckCanFire())
	{
		return;
	}
	if (CurrentWeaponFireMode == EWeaponFireMode::SEMI)
	{
		SimulateEachSingleShoot();
	}
	if (CurrentWeaponFireMode == EWeaponFireMode::SEMIAUTO)
	{
		InternalHandleSemiAutoFire();
	}
	if (CurrentWeaponFireMode == EWeaponFireMode::FULLAUTO)
	{
		InternalHandleBurstFire();
	}
}

void AINSWeaponBase::InspectWeapon()
{

}

void AINSWeaponBase::SetupWeaponMeshRenderings()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetNetMode() == ENetMode::NM_DedicatedServer)
		{
			WeaponMesh1PComp->SetHiddenInGame(true);
			WeaponMesh3PComp->SetHiddenInGame(true);
			WeaponMesh1PComp->SetComponentTickEnabled(false);
			WeaponMesh3PComp->SetComponentTickEnabled(false);
			WeaponMesh1PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			WeaponMesh3PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			WeaponMesh1PComp->SetCastShadow(false);
			WeaponMesh1PComp->bCastDynamicShadow = false;
			WeaponMesh3PComp->SetCastShadow(false);
			WeaponMesh3PComp->bCastDynamicShadow = false;
		}
		else if (GetNetMode() == ENetMode::NM_Standalone||GetNetMode()==ENetMode::NM_ListenServer)
		{
			//const APlayerController* const MyPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			WeaponMesh1PComp->SetHiddenInGame(false);
			WeaponMesh3PComp->SetHiddenInGame(true);
			WeaponMesh1PComp->SetComponentTickEnabled(true);
			WeaponMesh3PComp->SetComponentTickEnabled(false);
			WeaponMesh1PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			WeaponMesh3PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			WeaponMesh1PComp->SetCastShadow(true);
			WeaponMesh1PComp->bCastDynamicShadow = false;
			WeaponMesh3PComp->SetCastShadow(false);
			WeaponMesh3PComp->bCastDynamicShadow = false;
		}
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		WeaponMesh1PComp->SetHiddenInGame(false);
		WeaponMesh3PComp->SetHiddenInGame(true);
		WeaponMesh1PComp->SetComponentTickEnabled(true);
		WeaponMesh3PComp->SetComponentTickEnabled(false);
		WeaponMesh1PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		WeaponMesh3PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		WeaponMesh1PComp->SetCastShadow(true);
		WeaponMesh1PComp->bCastDynamicShadow = false;
		WeaponMesh3PComp->SetCastShadow(false);
		WeaponMesh3PComp->bCastDynamicShadow = false;
	}
	else
	{
		WeaponMesh1PComp->SetHiddenInGame(true);
		WeaponMesh3PComp->SetHiddenInGame(false);
		WeaponMesh1PComp->SetComponentTickEnabled(false);
		WeaponMesh3PComp->SetComponentTickEnabled(true);
		WeaponMesh1PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		WeaponMesh3PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		WeaponMesh1PComp->SetCastShadow(false);
		WeaponMesh1PComp->bCastDynamicShadow = false;
		WeaponMesh3PComp->SetCastShadow(false);
		WeaponMesh3PComp->bCastDynamicShadow = false;
	}
}

void AINSWeaponBase::WeaponGoToIdleState()
{

}

void AINSWeaponBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	InitWeaponAttachmentSlots();
}

void AINSWeaponBase::InitWeaponAttachmentSlots()
{
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::Muzzle, FWeaponAttachmentSlot(EWeaponAttachmentType::MUZZLE, true));
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::Sight, FWeaponAttachmentSlot(EWeaponAttachmentType::SIGHT, true));
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::UnderBarrel, FWeaponAttachmentSlot(EWeaponAttachmentType::UNDERBARREL, true));
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::LeftRail, FWeaponAttachmentSlot(EWeaponAttachmentType::LEFTRAIL, true));
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::rightRail, FWeaponAttachmentSlot(EWeaponAttachmentType::RIGHTRAIL, true));
}


void AINSWeaponBase::GetADSSightTransform(FTransform& OutTransform)
{
	if (WeaponMesh1PComp)
	{
		OutTransform = WeaponMesh1PComp->GetSocketTransform(TEXT("M68Carryhandle"));
	}
}

void AINSWeaponBase::GetWeaponAttachmentSlotStruct(FName SlotName, FWeaponAttachmentSlot& OutWeaponAttachmentSlot)
{
	FWeaponAttachmentSlot* TargetAttachmentSlot = WeaponAttachementSlots.Find(SlotName);
	if (!TargetAttachmentSlot)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("Weapon Slot Named %s Not found!"));
		return;
	}
	OutWeaponAttachmentSlot = *TargetAttachmentSlot;
}

void AINSWeaponBase::SpawnProjectile(FVector SpawnLoc, FVector SpawnDir, float TimeBetweenShots)
{
	FTransform ProjectileSpawnTransform;
	ProjectileSpawnTransform.SetLocation(SpawnLoc + SpawnDir * 20.f);
	ProjectileSpawnTransform.SetRotation(SpawnDir.ToOrientationQuat());
	AINSProjectile* SpawnedProjectile = GetWorld()->SpawnActorDeferred<AINSProjectile>(
		ProjectileClass,
		ProjectileSpawnTransform,
		GetOwnerCharacter()->GetController(),
		GetOwnerCharacter(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (SpawnedProjectile)
	{
		SpawnedProjectile->SetOwnerWeapon(this);
		SpawnedProjectile->SetIsFakeProjectile(false);
		SpawnedProjectile->SetMuzzleSpeed(WeaponConfigData.MuzzleSpeed);
		//SpawnedProjectile->SetMuzzleSpeed(500.f);
		SpawnedProjectile->SetCurrentPenetrateCount(0);
		SpawnedProjectile->SetInstigatedPlayer(Cast<AController>(GetOwner()));
		UGameplayStatics::FinishSpawningActor(SpawnedProjectile, ProjectileSpawnTransform);
		GetWorldTimerManager().SetTimer(ResetFireStateTimer, this, &AINSWeaponBase::ResetFireState, 1.f, false, WeaponConfigData.TimeBetweenShots);
		ConsumeAmmo();
	}
}

void AINSWeaponBase::ServerSpawnProjectile_Implementation(FVector SpawnLoc, FVector SpawnDir, float TimeBetweenShots)
{
	SpawnProjectile(SpawnLoc, SpawnDir, TimeBetweenShots);
}

bool AINSWeaponBase::ServerSpawnProjectile_Validate(FVector SpawnLoc, FVector SpawnDir, float TimeBetweenShots)
{
	return true;
}

void AINSWeaponBase::SetOwnerCharacter(class AINSCharacter* NewOwnerCharacter)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		this->OwnerCharacter = NewOwnerCharacter;
		OnRep_OwnerCharacter();
		SetWeaponState(EWeaponState::EQUIPPING);
		OnRep_CurrentWeaponState();
	}
}

void AINSWeaponBase::ServerSetWeaponState_Implementation(EWeaponState NewWeaponState)
{
	SetWeaponState(NewWeaponState);
}

bool AINSWeaponBase::ServerSetWeaponState_Validate(EWeaponState NewWeaponState)
{
	return true;
}


FORCEINLINE class UINSWeaponAnimInstance* AINSWeaponBase::GetWeapon1PAnimInstance()
{
	return Cast<UINSWeaponAnimInstance>(WeaponMesh1PComp->AnimScriptInstance);
}

FORCEINLINE class UINSWeaponAnimInstance* AINSWeaponBase::GetWeapon3pAnimINstance()
{
	return Cast<UINSWeaponAnimInstance>(WeaponMesh3PComp->AnimScriptInstance);
}

FTransform AINSWeaponBase::GetSightsTransform() const
{
	if (WeaponMesh1PComp)
	{
		return WeaponMesh1PComp->GetSocketTransform(TEXT("M68Carryhandle"));
	}
	return FTransform();
}

bool AINSWeaponBase::CheckScanTraceRange()
{
	FVector TraceStart(ForceInit);
	FVector TraceDir(ForceInit);
	FVector TraceEnd(ForceInit);
	if (GetOwnerCharacter()->IsLocallyControlled())
	{
		TraceDir = WeaponMesh1PComp->GetMuzzleForwardVector();
		TraceStart = WeaponMesh1PComp->GetMuzzleLocation();
		TraceEnd = TraceStart + TraceDir * WeaponConfigData.ScanTraceRange;
	}
	else
	{
		TraceDir = WeaponMesh3PComp->GetMuzzleForwardVector();
		TraceStart  = WeaponMesh3PComp->GetMuzzleLocation();
		TraceEnd = TraceStart + TraceDir * WeaponConfigData.ScanTraceRange;
	}
	FHitResult TraceHit(ForceInit);
	FCollisionQueryParams ScanTraceQuery;
	ScanTraceQuery.AddIgnoredActor(this);
	ScanTraceQuery.bReturnPhysicalMaterial = false;
	GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Camera, ScanTraceQuery);
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	if (bShowDebugTrace)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2.f);
		DrawDebugSphere(GetWorld(), TraceStart, 10.f, 8, FColor::Red, false, 2.f);
		DrawDebugSphere(GetWorld(), TraceEnd, 10.f, 8, FColor::Red, false, 2.f);
	}

#endif
	if (TraceHit.bBlockingHit)
	{
		return true;
	}
	return false;
}

void AINSWeaponBase::OnRep_CurrentWeaponState()
{
	if (CurrentWeaponState == EWeaponState::EQUIPPING)
	{
		OnWeaponOutIdle.Broadcast();
		OnWeaponStartEquip.Broadcast(bForeGripEquipt);
	}

	if (CurrentWeaponState == EWeaponState::RELOADIND)
	{
		if (GetOwnerCharacter()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			AINSPlayerCharacter* const PlayerCharacter = GetOwnerCharacter<AINSPlayerCharacter>();
			if (PlayerCharacter)
			{
				PlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
				PlayerCharacter->Get1PAnimInstance()->PlayReloadAnim(bForeGripEquipt, bDryReload);
				GetWeapon1PAnimInstance()->PlayReloadAnim(bForeGripEquipt, bDryReload);
			}
		}
		else if (GetOwnerCharacter()->GetLocalRole() == ROLE_SimulatedProxy)
		{
			AINSPlayerCharacter* const PlayerCharacter = GetOwnerCharacter<AINSPlayerCharacter>();
			if (PlayerCharacter)
			{
				PlayerCharacter->Get3PAnimInstance()->PlayReloadAnim(bForeGripEquipt, bDryReload);
				GetWeapon3pAnimINstance()->PlayReloadAnim(bForeGripEquipt, bDryReload);
			}
		}
		if (GetNetMode() != ENetMode::NM_DedicatedServer)
		{
			UINSCharacterAudioComponent* CharacterAudioComp = GetOwnerCharacter()->GetCharacterAudioComp();
			if (CharacterAudioComp)
			{
				CharacterAudioComp->OnWeaponReload();
			}
		}
	}

	else if (CurrentWeaponState == EWeaponState::FIREMODESWITCHING)
	{
		OnWeaponOutIdle.Broadcast();
		OnWeaponSwitchFireMode.Broadcast(bForeGripEquipt);
	}
}

void AINSWeaponBase::OnRep_AimWeapon()
{
	if (bisAiming)
	{
		OnWeaponAim.Broadcast();
		OnWeaponOutIdle.Broadcast();
		if (GetIsOwnerLocal() || GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			WeaponSpreadData.CurrentWeaponSpreadMax = WeaponSpreadData.DefaultWeaponSpreadMax * 0.2f;
			WeaponSpreadData.CurrentWeaponSpreadMin = 0.f;
			AINSPlayerController* PlayerController = OwnerCharacter->GetController() == nullptr ? 
				nullptr : CastChecked<AINSPlayerController>(OwnerCharacter->GetController());
			if (PlayerController)
			{
				class AINSPlayerCameraManager* PlayerCameraManager = CastChecked<AINSPlayerCameraManager>(PlayerController->PlayerCameraManager);
				if (PlayerCameraManager)
				{
					PlayerCameraManager->OnAim(AimTime);
				}
				PlayerController->GetINSPlayerCharacter()->UpdateADSHandsIkOffset();
			}
		}
	}
	else
	{
		OnStopWeaponAim.Broadcast();
		OnWeaponOutIdle.Broadcast();
		if (GetIsOwnerLocal() || GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			WeaponSpreadData.CurrentWeaponSpreadMax = WeaponSpreadData.DefaultWeaponSpreadMax;
			WeaponSpreadData.CurrentWeaponSpreadMin = WeaponSpreadData.DefaultWeaponSpread;
			AINSPlayerController* PlayerController = OwnerCharacter->GetController() == nullptr ?
				nullptr : CastChecked<AINSPlayerController>(OwnerCharacter->GetController());
			if (PlayerController)
			{
				class AINSPlayerCameraManager* PlayerCameraManager = CastChecked<AINSPlayerCameraManager>(PlayerController->PlayerCameraManager);
				if (PlayerCameraManager)
				{
					PlayerCameraManager->OnStopAim(AimTime);
				}
			}
		}
	}
}

void AINSWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	//SetupWeaponMeshRenderings();
}

void AINSWeaponBase::SimulateWeaponFireFX()
{
	USoundCue* SelectdFireSound = nullptr;
	UParticleSystem* SelectFireParticleTemplate = nullptr;
	UINSWeaponMeshComponent* SelectedFXAttachParent = nullptr;
	if (GetIsOwnerLocal())
	{
		SelectdFireSound = FireSound1P;
		SelectFireParticleTemplate = FireParticle1P;
		SelectedFXAttachParent = WeaponMesh1PComp;
		const AINSPlayerController* PlayerController = CastChecked<AINSPlayerController>(GetOwner());

	}
	else
	{
		SelectdFireSound = FireSound3P;
		SelectFireParticleTemplate = FireParticle3P;
		SelectedFXAttachParent = WeaponMesh3PComp;
	}
	if (SelectFireParticleTemplate)
	{
		WeaponParticleComp = UGameplayStatics::SpawnEmitterAttached(SelectFireParticleTemplate,
			SelectedFXAttachParent,
			SelectedFXAttachParent->GetWeaponSockets().MuzzleFlashSocket);
	}
	if (SelectdFireSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SelectdFireSound, SelectedFXAttachParent->GetMuzzleLocation(), SelectedFXAttachParent->GetMuzzleRotation());
	}
	CastShell();
}

void AINSWeaponBase::CastShell()
{
	if (!ProjectileShellClass || !GetOwnerCharacter())
	{
		return;
	}
	const UINSWeaponMeshComponent* SelectedMesh = GetIsOwnerLocal() ? WeaponMesh1PComp : WeaponMesh3PComp;
	FTransform ShellSpawnTran = SelectedMesh->GetShellSpawnTransform();
	AINSProjectileShell* ShellActor = GetWorld()->SpawnActorDeferred<AINSProjectileShell>(ProjectileShellClass, ShellSpawnTran, this, GetOwnerCharacter());
	if (ShellActor)
	{
		UGameplayStatics::FinishSpawningActor(ShellActor, ShellSpawnTran);
	}
}

void AINSWeaponBase::SimulateScanTrace(FHitResult& Hit)
{
	AINSPlayerController* OwnerPlayer = CastChecked<AINSPlayerController>(GetOwner());
	if (!OwnerPlayer)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("AINSWeaponBase::SimulateScanTrace::this Weapon :%s has no owner player,abort"), *GetName());
		return;
	}
	const AINSPlayerCharacter* const PlayerCharacter = OwnerPlayer->GetINSPlayerCharacter();
	if (PlayerCharacter)
	{
		FVector ViewLoc(ForceInit);
		FRotator ViewRot(ForceInit);
		OwnerPlayer->GetPlayerViewPoint(ViewLoc, ViewRot);
		FVector TraceDir = ViewRot.Vector();
		//sightly move forward  the trace start location to avoid hit selves
		FVector TraceStart = ViewLoc + TraceDir * 100.f;
		const float TraceRange = 10000.f;
		const FVector TraceEnd = TraceStart + TraceDir * TraceRange;
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Camera);
		if (Hit.bBlockingHit)
		{
//			UE_LOG(LogINSWeapon, Warning, TEXT("center trace hit result,thing be hit :%s,Hit component %s"), *Hit.GetActor()->GetName(), *(Hit.GetComponent()->GetName()));
		}
		//draw debug line when play or test with editor
#if WITH_EDITORONLY_DATA
		if (bShowDebugTrace && PlayerCharacter->IsLocallyControlled())
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 3.0f);
		}
#endif
	}
}

void AINSWeaponBase::AdjustProjectileDir(FVector& OutDir)
{
	if (!GetOwner())
	{
		OutDir = FVector::ZeroVector;
	}
	else
	{
		FHitResult ScanTraceHit(ForceInit);
		SimulateScanTrace(ScanTraceHit);
		if (ScanTraceHit.bBlockingHit)
		{
			OutDir = ScanTraceHit.Location - WeaponMesh1PComp->GetMuzzleLocation();
		}
		else
		{
			const AINSPlayerController* const PlayerController = Cast<AINSPlayerController>(GetOwner());
			if (PlayerController)
			{
				FRotator ViewRot(ForceInit);
				FVector ViewLoc(ForceInit);
				PlayerController->GetPlayerViewPoint(ViewLoc, ViewRot);
				OutDir = ViewRot.Vector();
			}
		}
	}
}

bool AINSWeaponBase::CheckCanFire()
{
	AINSPlayerController* const PlayerController = GetOwnerPlayer<AINSPlayerController>();
	const AINSPlayerCharacter* const PlayerCharacter = PlayerController == nullptr ? nullptr : PlayerController->GetINSPlayerCharacter();
	if (!PlayerController)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no owner player , can't fire"), *GetName());
		return false;
	}
	if (!PlayerCharacter)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no owner player character , can't fire"), *GetName());
		return false;
	}
	if (PlayerCharacter->GetIsCharacterDead())
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s and it's owner charcter is dead, cant't fire "), *GetName());
		return false;
	}
	else if (CurrentClipAmmo == 0)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s current clip is empty, can't fire "), *GetName());
		return false;
	}
	else if (CurrentWeaponState == EWeaponState::RELOADIND)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is reloading,can't fire "), *GetName());
		return false;
	}

	else if (CurrentWeaponState == EWeaponState::FIREMODESWITCHING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is switching fire mode,can't fire "), *GetName());
		return false;
	}

	else if (CurrentWeaponState == EWeaponState::EQUIPPING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is equiping,can't fire "), *GetName());
		return false;
	}

	else if (CurrentWeaponState == EWeaponState::UNEQUIPING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is un-equiping,can't fire "), *GetName());
		return false;
	}

	else if (CurrentWeaponState == EWeaponState::UNEQUIPED)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is un-equiped,can't fire "), *GetName());
		return false;
	}

	else if ((GetWorld()->GetTimeSeconds() - LastFireTime) < WeaponConfigData.TimeBetweenShots)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s fire interval not allowd,can't fire"), *GetName());
		return false;
	}
	return true;
}

void AINSWeaponBase::InternalHandleSemiAutoFire()
{
	if (GetLocalRole()==ROLE_Authority && CheckCanFire())
	{
		GetWorldTimerManager().SetTimer(WeaponSemiAutoTimerHandle, this, &AINSWeaponBase::SimulateEachSingleShoot, WeaponConfigData.TimeBetweenShots, true, 0.f);
	}
}

void AINSWeaponBase::InternalHandleBurstFire()
{
	if (GetLocalRole()==ROLE_Authority && CheckCanFire())
	{
		GetWorldTimerManager().SetTimer(WeaponBurstTimerHandle, this, &AINSWeaponBase::SimulateEachSingleShoot, WeaponConfigData.TimeBetweenShots, true, 0.f);
	}
}

void AINSWeaponBase::StopWeaponFire()
{
	GetWorldTimerManager().ClearTimer(WeaponBurstTimerHandle);
	GetWorldTimerManager().ClearTimer(WeaponSemiAutoTimerHandle);
	SetWeaponState(EWeaponState::IDLE);
}

void AINSWeaponBase::ResetFireState()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetWeaponState(EWeaponState::IDLE);
	}
}

bool AINSWeaponBase::CheckCanReload()
{
	AINSPlayerController* const PlayerController = GetOwnerPlayer<AINSPlayerController>();
	const AINSPlayerCharacter* const PlayerCharacter = PlayerController == nullptr ? nullptr : PlayerController->GetINSPlayerCharacter();
	if (!PlayerController)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no owner player , can't reload"), *GetName());
		return false;
	}
	if (!PlayerCharacter)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no owner player character , can't reload"), *GetName());
		return false;
	}
	if (PlayerCharacter->GetIsCharacterDead())
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s and it's owner charcter is dead, cant't reload "), *GetName());
		return false;
	}
	else if (CurrentClipAmmo == WeaponConfigData.AmmoPerClip)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s current clip is full, No Need reloading"), *GetName());
		return false;
	}
	else if (AmmoLeft <= 0)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no ammo left, cant't reload"), *GetName());
		return false;
	}
	else if (CurrentWeaponState == EWeaponState::RELOADIND)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is Already reloading,can't Reload"), *GetName());
		return false;
	}

	else if (CurrentWeaponState == EWeaponState::FIREMODESWITCHING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is switching fire mode,can't Reload "), *GetName());
		return false;
	}

	else if (CurrentWeaponState == EWeaponState::EQUIPPING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is equiping,can't Reload "), *GetName());
		return false;
	}

	else if (CurrentWeaponState == EWeaponState::UNEQUIPING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is un-equiping,can't Reload "), *GetName());
		return false;
	}
	else if (CurrentWeaponState == EWeaponState::UNEQUIPED)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is un-equiped,can't Reload "), *GetName());
		return false;
	}
	return true;
}

void AINSWeaponBase::SimulateEachSingleShoot()
{
	RepWeaponFireCount++;
	FTransform ProjectileSpawnTransform;
	FVector AdjustDir(ForceInit);
	AdjustProjectileDir(AdjustDir);
	FVector SpreadDir(ForceInit);
	AddWeaponSpread(SpreadDir, AdjustDir);
	if (GetLocalRole() == ROLE_Authority)
	{
		SetWeaponState(EWeaponState::FIRING);
		SpawnProjectile(WeaponMesh1PComp->GetMuzzleLocation(), SpreadDir, GetWorld()->GetTimeSeconds() - LastFireTime);
		
	}
	if (CurrentWeaponFireMode == EWeaponFireMode::SEMIAUTO)
	{
		SemiAutoCurrentRoundCount++;
		if (SemiAutoCurrentRoundCount == 3)
		{
			GetWorldTimerManager().ClearTimer(WeaponSemiAutoTimerHandle);
		}
	}
	if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
	{
		OnRep_WeaponFireCount();
	}
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	if (bShowDebugTrace && GetOwnerCharacter()->IsLocallyControlled())
	{
		const FVector TraceStart = WeaponMesh1PComp->GetMuzzleLocation();
		const FVector TraceEnd = TraceStart + SpreadDir * 1000;
		DrawDebugLine(GetWorld(), WeaponMesh1PComp->GetMuzzleLocation(), TraceEnd, FColor::Blue, false, 2.0f);
		DrawDebugSphere(GetWorld(), ProjectileSpawnTransform.GetLocation(), 5.f, 5, FColor::Red, false, 10.f);
	}
#endif
}

void AINSWeaponBase::AddWeaponSpread(FVector& OutSpreadDir, FVector& BaseDirection)
{
	const int32 RandomSeed = FMath::Rand();
	const FRandomStream WeaponRandomStream(RandomSeed);
	const float RandomFloat = FMath::RandRange(0.5f, 1.5f);
	const float ConeHalfAngle = FMath::DegreesToRadians(WeaponSpreadData.CurrentWeaponSpread * RandomFloat);
	OutSpreadDir = WeaponRandomStream.VRandCone(BaseDirection, ConeHalfAngle, ConeHalfAngle);
}


void AINSWeaponBase::CalculateAmmoAfterReload()
{
	if (bInfinitAmmo)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s infinit ammo mode has enabled,reloading will consumes no ammo "), *GetName());
		CurrentClipAmmo = WeaponConfigData.AmmoPerClip;
	}
	else {
		const int32 AmmoPerClip = WeaponConfigData.AmmoPerClip;
		const int32 DeltaAmmoSize = WeaponConfigData.AmmoPerClip - CurrentClipAmmo;
		if (AmmoLeft >= DeltaAmmoSize)
		{
			CurrentClipAmmo += DeltaAmmoSize;
			AmmoLeft -= DeltaAmmoSize;
		}
		else if (AmmoLeft < DeltaAmmoSize)
		{
			CurrentClipAmmo += AmmoLeft;
			AmmoLeft = 0;
		}
	}
}


void AINSWeaponBase::ConsumeAmmo()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (CurrentClipAmmo > 0)
		{
			CurrentClipAmmo = FMath::Clamp<int32>(CurrentClipAmmo--, 0, CurrentClipAmmo);
		}
		else if (CurrentClipAmmo == 0)
		{
			StopWeaponFire();
			bDryReload = true;
			StartReloadWeapon();
		}
	}
}

void AINSWeaponBase::OnRep_WeaponFireCount()
{
	if (GetOwnerCharacter() && !GetOwnerCharacter()->GetIsCharacterDead())
	{
		OnWeaponOutIdle.Broadcast();
		OnWeaponEachFire.Broadcast(bForeGripEquipt, CurrentClipAmmo == 0);
		SimulateWeaponFireFX();
		AINSPlayerController* PlayerController = Cast<AINSPlayerController>(GetOwnerCharacter()->GetController());
		if (PlayerController)
		{
			PlayerController->ClientPlayCameraShake(FireCameraShakingClass);
		}
	}
}

void AINSWeaponBase::OnRep_OwnerCharacter()
{
	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		const AINSPlayerCharacter* const PlayerCharacter = Cast<AINSPlayerCharacter>(GetOwnerCharacter());
		if (PlayerCharacter)
		{
			if (PlayerCharacter->IsLocallyControlled())
			{
				AINSHUDBase* const PlayerHud = Cast<AINSHUDBase>(Cast<AINSPlayerController>(GetOwnerCharacter()->GetController())->GetHUD());
				if (PlayerHud)
				{
					PlayerHud->SetCurrentWeapon(this);
				}
			}
		}
	}
}

void AINSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSWeaponBase, CurrentWeaponState);
	DOREPLIFETIME(AINSWeaponBase, RepWeaponFireCount);
	DOREPLIFETIME(AINSWeaponBase, OwnerCharacter);
	DOREPLIFETIME(AINSWeaponBase, bisAiming);
	DOREPLIFETIME(AINSWeaponBase, bDryReload);
	DOREPLIFETIME(AINSWeaponBase, bWantsToEquip);
	DOREPLIFETIME_CONDITION(AINSWeaponBase, CurrentClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AINSWeaponBase, AmmoLeft, COND_OwnerOnly);
}


void AINSWeaponBase::OwnerPlayCameraShake()
{
	if (GetOwner())
	{
		AINSPlayerController* const OwnerController = GetOwnerPlayer<AINSPlayerController>();
		if (OwnerController)
		{
			OwnerController->ClientPlayCameraShake(FireCameraShakingClass);
		}
	}
}

void AINSWeaponBase::UpdateWeaponSpread(float DeltaTimeSeconds)
{
	if (GetOwner() && GetOwner()->GetLocalRole() != ROLE_SimulatedProxy)
	{
		if (CurrentWeaponState == EWeaponState::IDLE)
		{
			WeaponSpreadData.CurrentWeaponSpread = FMath::Clamp<float>(WeaponSpreadData.CurrentWeaponSpread - DeltaTimeSeconds * 20.f,
				WeaponSpreadData.CurrentWeaponSpreadMin,
				WeaponSpreadData.CurrentWeaponSpread);
		}

		else if (CurrentWeaponState == EWeaponState::FIRING)
		{
			WeaponSpreadData.CurrentWeaponSpread = FMath::Clamp<float>(WeaponSpreadData.CurrentWeaponSpread + DeltaTimeSeconds * 40.f,
				WeaponSpreadData.CurrentWeaponSpread,
				WeaponSpreadData.CurrentWeaponSpreadMax);
		}

		if (GetOwnerCharacter() && GetOwnerCharacter()->GetIsCharacterMoving())
		{
			WeaponSpreadData.CurrentWeaponSpread = FMath::Clamp<float>(WeaponSpreadData.CurrentWeaponSpread + DeltaTimeSeconds * 40.f,
				WeaponSpreadData.CurrentWeaponSpread,
				WeaponSpreadData.CurrentWeaponSpreadMax);
		}
		else if (GetOwnerCharacter() && !GetOwnerCharacter()->GetIsCharacterMoving())
		{
			WeaponSpreadData.CurrentWeaponSpread = FMath::Clamp<float>(WeaponSpreadData.CurrentWeaponSpread - DeltaTimeSeconds * 40.f,
				WeaponSpreadData.CurrentWeaponSpreadMin,
				WeaponSpreadData.CurrentWeaponSpread);
		}
	}
}

void AINSWeaponBase::OnRep_CurrentFireMode()
{
	OnWeaponSwitchFireMode.Broadcast(bForeGripEquipt);
	OnWeaponOutIdle.Broadcast();
}

void AINSWeaponBase::OnRep_Equipping()
{

}

void AINSWeaponBase::OnRep_CurrentClipAmmo()
{
	if (!CurrentClipAmmo == 0)
	{
		return;
	}
	else
	{
		if (GetLocalRole() == ROLE_Authority ||
			GetLocalRole() == ROLE_AutonomousProxy&&
			GetNetMode()!=ENetMode::NM_DedicatedServer)
		{
			//OnClipEmpty.Broadcast(Cast<AController>(GetOwner()));
			if (GetOwner() && GetOwner()->GetClass() == AINSPlayerController::StaticClass())
			{
				AINSPlayerController* OwnerPlayer = Cast<AINSPlayerController>(GetOwner());
				if (OwnerPlayer)
				{
					OwnerPlayer->StopFire();
					OwnerPlayer->OnWeaponClipEmpty(OwnerPlayer);
// 					SetWeaponState(EWeaponState::RELOADIND);
// 										StartReloadWeapon();
				}
			}
		}
	}
}

void AINSWeaponBase::StartReloadWeapon()
{
	if (CheckCanReload())
	{
		if (GetOwnerCharacter()->GetIsAiming())
		{
			StopWeaponAim();
		}
		CurrentWeaponState = EWeaponState::RELOADIND;
		if (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			OnRep_CurrentWeaponState();
		}
	}
}

void AINSWeaponBase::ServerStartReloadWeapon_Implementation()
{
	StartReloadWeapon();
}

bool AINSWeaponBase::ServerStartReloadWeapon_Validate()
{
	return true;
}

void AINSWeaponBase::SetWeaponReady()
{
	CurrentWeaponState = EWeaponState::IDLE;
}

void AINSWeaponBase::StartEquipWeapon()
{
	SetWeaponState(EWeaponState::EQUIPPING);
	if (GetLocalRole() == ROLE_Authority && (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone))
	{
		OnRep_CurrentWeaponState();
	}
}

void AINSWeaponBase::SetWeaponMeshVisibility(bool WeaponMesh1pVisible, bool WeaponMesh3pVisible)
{
	WeaponMesh1PComp->SetHiddenInGame(WeaponMesh1pVisible);
	WeaponMesh3PComp->SetHiddenInGame(WeaponMesh3pVisible);
}

void AINSWeaponBase::ServerStartEquipWeapon_Implementation()
{
	StartEquipWeapon();
}

bool AINSWeaponBase::ServerStartEquipWeapon_Validate()
{
	return true;
}

void AINSWeaponBase::StartWeaponAim()
{
	if (CheckCanAim())
	{
		bisAiming = true;
		GetOwnerCharacter()->SetIsAiming(true);
		if (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			OnRep_AimWeapon();
		}
	}
}

void AINSWeaponBase::StopWeaponAim()
{
	bisAiming = false;
	GetOwnerCharacter()->SetIsAiming(false);
	if (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
	{
		OnRep_AimWeapon();
	}
}

bool AINSWeaponBase::CheckCanAim()
{
	const AINSPlayerController* PlayerController = nullptr;
	const AINSPlayerCharacter* PlayerCharacter = nullptr;
	if (!GetOwner())
	{
		return false;
	}
	if (GetOwner())
	{
		PlayerController = CastChecked<AINSPlayerController>(GetOwner());
	}
	if (!PlayerController)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no owner player , can't reload"), *GetName());
		return false;
	}
	return true;
}

void AINSWeaponBase::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_Owner();
	}
}

void AINSWeaponBase::UpdateRecoilVertically(float DeltaTimeSeconds, float RecoilAmount)
{
	if (GetOwnerCharacter())
	{
		Cast<AINSPlayerController>(GetOwnerCharacter()->GetController())->AddPitchInput(RecoilAmount*DeltaTimeSeconds);
	}
}

void AINSWeaponBase::UpdateRecoilHorizontally(float DeltaTimeSeconds, float RecoilAmount)
{
	if (GetOwnerCharacter())
	{
		RecoilAmount = FMath::RandRange(-RecoilAmount, RecoilAmount);
		Cast<AINSPlayerController>(GetOwnerCharacter()->GetController())->AddYawInput(RecoilAmount*DeltaTimeSeconds);
	}
}

FString AINSWeaponBase::GetWeaponReadableCurrentState()
{
	FString ReadableCurrentWeaponState;
	switch (CurrentWeaponState)
	{
	case EWeaponState::IDLE:ReadableCurrentWeaponState.Append("Idle");
		break;
	case EWeaponState::FIRING:ReadableCurrentWeaponState.Append("firing");
		break;
	case EWeaponState::RELOADIND:ReadableCurrentWeaponState.Append("reloading");
		break;
	case EWeaponState::UNEQUIPED:ReadableCurrentWeaponState.Append("unEquiped");
		break;
	case EWeaponState::EQUIPPING:ReadableCurrentWeaponState.Append("Equipping");
		break;
	case EWeaponState::FIREMODESWITCHING:ReadableCurrentWeaponState.Append("fire mode switching");
		break;
	case EWeaponState::UNEQUIPING:ReadableCurrentWeaponState.Append("unEquipping");
		break;
	case EWeaponState::EQUIPED:ReadableCurrentWeaponState.Append("equipped");
		break;
	default:
		break;
	}
	return ReadableCurrentWeaponState;
}

void AINSWeaponBase::FinishEquippingWeapon()
{
	SetWeaponReady();
	CalculateAmmoAfterReload();
}


void AINSWeaponBase::ServerFinishEquippingWeapon_Implementation()
{
	FinishEquippingWeapon();
	OnFinishReloadWeapon.Broadcast();
}

bool AINSWeaponBase::ServerFinishEquippingWeapon_Validate()
{
	return true;
}


void AINSWeaponBase::FinishReloadWeapon()
{
	SetWeaponReady();
	CalculateAmmoAfterReload();
	bDryReload = false;
}


bool AINSWeaponBase::GetIsOwnerLocal()
{
	if (OwnerCharacter)
	{
		return OwnerCharacter->IsLocallyControlled();
	}
	else if (GetOwner())
	{
		const class APlayerController* OwnerController = Cast<APlayerController>(GetOwner());
		if (OwnerController)
		{
			return OwnerController->IsLocalController();
		}
	}
	return false;
}

void AINSWeaponBase::ServerFinishReloadWeapon_Implementation()
{
	FinishReloadWeapon();
}

bool AINSWeaponBase::ServerFinishReloadWeapon_Validate()
{
	return true;
}

void AINSWeaponBase::StartSwitchFireMode()
{
	OnWeaponOutIdle.Broadcast();
	const uint8 AvailableFireModesNum = AvailableFireModes.Num();
	uint8 CurrentFireModeIndex = 0;
	uint8 NextFireModeIndex = 0;
	for (uint8 FireModeIndex = 0; FireModeIndex < AvailableFireModesNum; FireModeIndex++)
	{
		if (CurrentWeaponFireMode == AvailableFireModes[FireModeIndex])
		{
			CurrentFireModeIndex = FireModeIndex;
			NextFireModeIndex = CurrentFireModeIndex + 1;
			if (NextFireModeIndex > AvailableFireModesNum - 1) {
				NextFireModeIndex = 0;
			}
			CurrentWeaponFireMode = AvailableFireModes[NextFireModeIndex];
			if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
			{
				OnRep_CurrentFireMode();
			}
		}
	}
}

void AINSWeaponBase::FinishSwitchFireMode()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetWeaponState(EWeaponState::IDLE);
	}
	else
	{
		ServerSetWeaponState(EWeaponState::IDLE);
	}
}

void AINSWeaponBase::ServerFinishSwitchFireMode_Implementation()
{
	FinishSwitchFireMode();
}

bool AINSWeaponBase::ServerFinishSwitchFireMode_Validate()
{
	return true;
}
