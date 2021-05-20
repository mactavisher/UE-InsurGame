// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "INSCharacter/INSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "INSAnimation/INSCharacterAimInstance.h"
#include "Camera/CameraShake.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundCue.h"
#include "INSEffects/INSProjectileShell.h"
#include "INSProjectiles/INSProjectile.h"
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
#include "INSAssets/INSStaticAnimData.h"
#ifndef UWorld
#include "Engine/World.h"
#endif // !UWorld

#include "INSComponents/INSWeaponFireHandler.h"

DEFINE_LOG_CATEGORY(LogINSWeapon);

AINSWeaponBase::AINSWeaponBase(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	ItemType = EItemType::WEAPON;
	AvailableFireModes.Add(EWeaponFireMode::FULLAUTO);
	AvailableFireModes.Add(EWeaponFireMode::SEMI);
	CurrentWeaponFireMode = AvailableFireModes[0];
	CurrentClipAmmo = WeaponConfigData.AmmoPerClip;
	AmmoLeft = WeaponConfigData.MaxAmmo;
	CurrentWeaponState = EWeaponState::NONE;
	CurrentWeaponBasePoseType = EWeaponBasePoseType::ALTGRIP;
	LastFireTime = 0.f;
	RepWeaponFireCount = 0;
	bIsAimingWeapon = false;
	bInfinitAmmo = false;
	AimTime = 0.3f;
	RecoilVerticallyFactor = -3.f;
	RecoilHorizontallyFactor = 6.f;
	NetUpdateFrequency = 10.f;
	BaseHandsIk = FVector::ZeroVector;
	bDryReload = false;
	SetReplicateMovement(false);
	WeaponMesh1PComp = ObjectInitializer.CreateDefaultSubobject<UINSWeaponMeshComponent>(this, TEXT("WeaponMesh1PComp"));
	WeaponMesh3PComp = ObjectInitializer.CreateDefaultSubobject<UINSWeaponMeshComponent>(this, TEXT("WeaponMesh3PComp"));
	WeaponParticleComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticelSystemComp"));
	WeaponFireHandler = ObjectInitializer.CreateDefaultSubobject<UINSWeaponFireHandler>(this, TEXT("WeaponFireHandler"));
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
	WeaponAnimationClass = UINSStaticAnimData::StaticClass();
	CurrentWeaponZoomState = EZoomState::ZOMMEDOUT;
#if WITH_EDITORONLY_DATA
	bShowDebugTrace = false;
#endif
	ProbeSize = 10.f;
	ZoomedInEventTriggered = false;
	ZoomedOutEventTriggered = false;
}

void AINSWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentWeaponState == EWeaponState::IDLE)
	{
		
	}
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		UpdateWeaponSpread(DeltaTime);
		if (CurrentWeaponState == EWeaponState::FIRING)
		{
			UpdateRecoilHorizontally(DeltaTime, GetRecoilHorizontallyFactor());
			UpdateRecoilVertically(DeltaTime, GetRecoilVerticallyFactor());
		}
	}
	UpdateWeaponCollide();
	UpdateADSStatus(DeltaTime);
}

void AINSWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	WeaponFireHandler->SetOwnerWeapon(this);
}

void AINSWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	SetupWeaponMeshRenderings();
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_WeaponBasePoseType();
	}
}

void AINSWeaponBase::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	DOREPLIFETIME_ACTIVE_OVERRIDE(AINSWeaponBase, ScanTraceHitLoc, !ScanTraceHitLoc.IsZero());
}

void AINSWeaponBase::StartWeaponFire()
{
	if (WeaponFireHandler) 
	{
		WeaponFireHandler->BeginWeaponFire(CurrentWeaponFireMode);
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
		else if (IsNetMode(NM_Standalone) || IsNetMode(NM_ListenServer))
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
	/*WeaponMesh1PComp->SetOnlyOwnerSee(true);
	WeaponMesh3PComp->SetOwnerNoSee(true);*/
}

void AINSWeaponBase::WeaponGoToIdleState()
{

}

void AINSWeaponBase::PlayWeaponReloadAnim()
{
	
}

void AINSWeaponBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	InitWeaponAttachmentSlots();
	WeaponAnimation = NewObject<UINSStaticAnimData>(this, WeaponAnimationClass);
}

void AINSWeaponBase::InitWeaponAttachmentSlots()
{
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::Muzzle, FWeaponAttachmentSlot(EWeaponAttachmentType::MUZZLE, true));
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::Sight, FWeaponAttachmentSlot(EWeaponAttachmentType::SIGHT, true));
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::UnderBarrel, FWeaponAttachmentSlot(EWeaponAttachmentType::UNDER_BARREL, true));
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::LeftRail, FWeaponAttachmentSlot(EWeaponAttachmentType::LEFT_RAIL, true));
	WeaponAttachementSlots.Add(WeaponAttachmentSlotName::rightRail, FWeaponAttachmentSlot(EWeaponAttachmentType::RIGHT_RAIL, true));
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

void AINSWeaponBase::SpawnProjectile(FVector SpawnLoc, FVector SpawnDir)
{
	if (HasAuthority())
	{
		FTransform ProjectileSpawnTransform;
		ProjectileSpawnTransform.SetLocation(SpawnLoc + SpawnDir * 20.f);
		ProjectileSpawnTransform.SetRotation(SpawnDir.ToOrientationQuat());
		UWorld* const world = GetWorld();
		if (world == nullptr)
		{
			UE_LOG(LogINSWeapon, Warning, TEXT("trying to spawn a weapon projectile but world context object is null"));
			return;
		}
		FHitResult ScanTraceHitResult(ForceInit);
		FCollisionQueryParams WeaponScanTraceParam;
		WeaponScanTraceParam.AddIgnoredActor(this);
		WeaponScanTraceParam.AddIgnoredActor(GetOwnerCharacter());
		world->LineTraceSingleByChannel(ScanTraceHitResult, SpawnLoc, SpawnLoc + SpawnDir * WeaponConfigData.ScanTraceRange,ECC_Camera,WeaponScanTraceParam);
		if (ScanTraceHitResult.bBlockingHit)
		{
			ScanTraceHitLoc = ScanTraceHitResult.Location;
		}
		AINSProjectile* SpawnedProjectile = world->SpawnActorDeferred<AINSProjectile>(
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
			SpawnedProjectile->SetCurrentPenetrateCount(0);
			SpawnedProjectile->SetInstigatedPlayer(Cast<AController>(GetOwner()));
			SpawnedProjectile->SetScanTraceProjectile(ScanTraceHitResult.bBlockingHit);
			UGameplayStatics::FinishSpawningActor(SpawnedProjectile, ProjectileSpawnTransform);
			ConsumeAmmo();
		}
	}
}

void AINSWeaponBase::ServerSpawnProjectile_Implementation(FVector SpawnLoc, FVector SpawnDir)
{
	SpawnProjectile(SpawnLoc, SpawnDir);
}

bool AINSWeaponBase::ServerSpawnProjectile_Validate(FVector SpawnLoc, FVector SpawnDir)
{
	return true;
}

void AINSWeaponBase::SetOwnerCharacter(class AINSCharacter* NewOwnerCharacter)
{
	if (HasAuthority())
	{
		this->OwnerCharacter = NewOwnerCharacter;
		OnRep_OwnerCharacter();
		SetWeaponState(EWeaponState::EQUIPPING);
	}
}

void AINSWeaponBase::SetWeaponState(EWeaponState NewWeaponState)
{
	if (HasAuthority())
	{
		CurrentWeaponState = NewWeaponState;
		OnRep_CurrentWeaponState();
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSetWeaponState(NewWeaponState);
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
	
	return WeaponMesh1PComp->GetSocketTransform(SightAlignerSocketName, ERelativeTransformSpace::RTS_World);
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
		TraceStart = WeaponMesh3PComp->GetMuzzleLocation();
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


void AINSWeaponBase::FireShot(FVector FireLoc, FRotator ShotRot)
{
	if (HasAuthority())
	{
		SpawnProjectile(FireLoc, ShotRot.Vector());
		RepWeaponFireCount++;
		if (IsNetMode(NM_Standalone) || IsNetMode(NM_ListenServer))
		{
			OnRep_WeaponFireCount();
		}
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
		if (bShowDebugTrace && GetOwnerCharacter()->IsLocallyControlled())
		{
			const FVector TraceStart = WeaponMesh1PComp->GetMuzzleLocation();
			const FVector TraceEnd = TraceStart + ShotRot.Vector() * 1000;
			DrawDebugLine(GetWorld(), WeaponMesh1PComp->GetMuzzleLocation(), TraceEnd, FColor::Blue, false, 2.0f);
			DrawDebugSphere(GetWorld(), FireLoc, 5.f, 5, FColor::Red, false, 10.f);
		}
#endif
	}

	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerFireShot(FireLoc, ShotRot);
	}
}
	

void AINSWeaponBase::OnZoomedOut()
{
	if (IsNetMode(NM_ListenServer) || GetLocalRole() == ROLE_AutonomousProxy)
	{
		UGameplayStatics::SpawnSoundAttached(ADSOutSound, WeaponMesh1PComp);
	}
	else
	{
		UGameplayStatics::SpawnSoundAttached(ADSOutSound, WeaponMesh3PComp);
	}
}

void AINSWeaponBase::OnZoomedIn()
{
	if (IsNetMode(NM_ListenServer) || GetLocalRole() == ROLE_AutonomousProxy)
	{
		UGameplayStatics::SpawnSoundAttached(ADSInSound, WeaponMesh1PComp);
	}
	else
	{
		UGameplayStatics::SpawnSoundAttached(ADSInSound, WeaponMesh3PComp);
	}
}

void AINSWeaponBase::UpdateADSStatus(const float DeltaSeconds)
{
	const float InterpSpeed = 1.f / AimTime;
	if (bIsAimingWeapon)
	{
		ZoomedOutEventTriggered = false;
		ADSAlpha = FMath::Clamp<float>(ADSAlpha + InterpSpeed * DeltaSeconds, ADSAlpha, 1.f);
		if (!IsNetMode(NM_DedicatedServer))
		{
			CurrentWeaponZoomState = ADSAlpha == 1.f ? EZoomState::ZOOMED : EZoomState::ZOOMING;
			if (ADSAlpha > 0.8f && !ZoomedInEventTriggered)
			{
				OnZoomedIn();
				ZoomedInEventTriggered = true;
			}
		}
	}
	else
	{
		ZoomedInEventTriggered = false;
		ADSAlpha = FMath::Clamp<float>(ADSAlpha - InterpSpeed * DeltaSeconds, 0.f, ADSAlpha);
		if (!IsNetMode(NM_DedicatedServer))
		{
			CurrentWeaponZoomState = ADSAlpha == 0.f ? EZoomState::ZOMMEDOUT : EZoomState::ZOZMINGOUT;
			if (ADSAlpha < 0.8f && !ZoomedOutEventTriggered)
			{
				OnZoomedOut();
				ZoomedOutEventTriggered = true;
			}
		}
	}
}

void AINSWeaponBase::ServerFireShot_Implementation(FVector FireLoc, FRotator ShotRot)
{
	FireShot(FireLoc, ShotRot);
}


bool AINSWeaponBase::ServerFireShot_Validate(FVector FireLoc, FRotator ShotRot)
{
	return true;
}

bool AINSWeaponBase::IsSightAlignerExist() const
{
	return WeaponMesh1PComp->DoesSocketExist(SightAlignerSocketName);
}

void AINSWeaponBase::OnRep_CurrentWeaponState()
{
	AINSPlayerCharacter* const PlayerCharacter = GetOwnerCharacter<AINSPlayerCharacter>();
	//weapon equipping state replication
	if (CurrentWeaponState == EWeaponState::EQUIPPING)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy || GetLocalRole() == ROLE_Authority)
		{
			if (PlayerCharacter)
			{
				PlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
				PlayerCharacter->Get1PAnimInstance()->PlayWeaponStartEquipAnim();
				GetWeapon1PAnimInstance()->PlayWeaponStartEquipAnim();
			}
		}
		else if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			if (PlayerCharacter)
			{
				PlayerCharacter->Get3PAnimInstance()->PlayWeaponStartEquipAnim();
				GetWeapon3pAnimINstance()->PlayWeaponStartEquipAnim();
			}
		}
	}
	//weapon reloading state replication
	if (CurrentWeaponState == EWeaponState::RELOADIND)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy || GetLocalRole() == ROLE_Authority)
		{
			if (PlayerCharacter)
			{
				PlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
				PlayerCharacter->Get1PAnimInstance()->PlayReloadAnim(bDryReload);
				GetWeapon1PAnimInstance()->PlayReloadAnim(bDryReload);
			
			}
		}
		else if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			if (PlayerCharacter)
			{
				PlayerCharacter->Get3PAnimInstance()->PlayReloadAnim(bDryReload);
				PlayerCharacter->Get3PAnimInstance()->PlayReloadAnim(bDryReload);
				GetWeapon3pAnimINstance()->PlayReloadAnim(bDryReload);
			}
		}
		if (IsNetMode(NM_DedicatedServer)) {
			UINSCharacterAudioComponent* CharacterAudioComp = GetOwnerCharacter()->GetCharacterAudioComp();
			if (CharacterAudioComp)
			{
				CharacterAudioComp->OnWeaponReload();
			}
		}
	}
	//weapon fire mode switching state replication
	else if (CurrentWeaponState == EWeaponState::FIREMODESWITCHING)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy || GetLocalRole() == ROLE_Authority)
		{
			if (PlayerCharacter)
			{
				PlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
				PlayerCharacter->Get1PAnimInstance()->PlaySwitchFireModeAnim();
				GetWeapon1PAnimInstance()->PlaySwitchFireModeAnim();
			}
		}
		else if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			if (PlayerCharacter)
			{
				PlayerCharacter->Get3PAnimInstance()->PlaySwitchFireModeAnim();
				GetWeapon3pAnimINstance()->PlaySwitchFireModeAnim();
			}
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Weapon state replicated," + GetWeaponReadableCurrentState()));
}

void AINSWeaponBase::OnRep_AimWeapon()
{
	if (bIsAimingWeapon)
	{
		if (GetIsOwnerLocal() || GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			WeaponSpreadData.CurrentWeaponSpreadMax = WeaponSpreadData.DefaultWeaponSpreadMax * 0.2f;
			WeaponSpreadData.CurrentWeaponSpreadMin = 0.f;
		}
	}
	else
	{
		if (GetIsOwnerLocal() || GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			WeaponSpreadData.CurrentWeaponSpreadMax = WeaponSpreadData.DefaultWeaponSpreadMax;
			WeaponSpreadData.CurrentWeaponSpreadMin = WeaponSpreadData.DefaultWeaponSpread;
		}
	}
}

void AINSWeaponBase::OnRep_WeaponBasePoseType()
{
	FString Message;
	Message.Append("Weapon:");
	Message.Append(GetName());
	Message.Append("Base pose Type Replicated!Current Weapon base type is : ");
	switch (CurrentWeaponBasePoseType)
	{
	case EWeaponBasePoseType::ALTGRIP:Message.Append("Alt Grip");
		break;
	case EWeaponBasePoseType::FOREGRIP:Message.Append("Fore Grip");
		break;
	case EWeaponBasePoseType::DEFAULT:Message.Append("Default Grip");
		break;
	default:
		break;
	}
#if WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, Message);
#endif
	UE_LOG(LogINSWeapon, Log, TEXT("%s"),*Message);
	const UClass* CharacterClass = GetOwnerCharacter()->GetClass();
	if (CharacterClass->IsChildOf(AINSPlayerCharacter::StaticClass()))
	{
		AINSPlayerCharacter* PlayerCharacter = Cast<AINSPlayerCharacter>(GetOwnerCharacter());
		PlayerCharacter->Get1PAnimInstance()->SetWeaponBasePoseType(CurrentWeaponBasePoseType);
		PlayerCharacter->Get3PAnimInstance()->SetWeaponBasePoseType(CurrentWeaponBasePoseType);
	}
}

void AINSWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	//SetupWeaponMeshRenderings();
}

void AINSWeaponBase::SimulateWeaponFireFX()
{
	//spawn shooting sound
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

	//spawn muzzle emmiter
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

	//cast projectile shell
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
		if (Hit.bBlockingHit&&Hit.GetActor())
		{
			UE_LOG(LogINSWeapon, Warning, TEXT("center trace hit result,thing be hit :%s,Hit component %s"), *Hit.GetActor()->GetName(), *(Hit.GetComponent()->GetName()));
		}
#if WITH_EDITORONLY_DATA
		if (bShowDebugTrace && PlayerCharacter->IsLocallyControlled())
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 3.0f);
		}
#endif
	}
}

void AINSWeaponBase::GetFireDir(FVector& OutDir)
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
	AINSPlayerController* PlayerController = Cast<AINSPlayerController>(GetOwner());
	AINSPlayerCharacter*PlayerCharacter = PlayerController == nullptr ? nullptr : PlayerController->GetINSPlayerCharacter();
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
	if (PlayerCharacter->GetIsDead())
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

void AINSWeaponBase::StopWeaponFire()
{
	WeaponFireHandler->StopWeaponFire();
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
	if (PlayerCharacter->GetIsDead())
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

void AINSWeaponBase::UpdateWeaponCollide()
{
	if (GetOwnerCharacter()&& !GetOwnerCharacter()->GetIsDead()&&GetLocalRole() == ROLE_AutonomousProxy)
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(this->GetOwner());
		QueryParams.AddIgnoredComponent(WeaponMesh1PComp);
		QueryParams.AddIgnoredComponent(WeaponMesh3PComp);
		QueryParams.AddIgnoredActor(this->GetOwnerCharacter());
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		QueryParams.bDebugQuery = true;
#endif
		FHitResult Result;
		bool bCollide = GetWorld()->SweepSingleByChannel(Result, WeaponMesh1PComp->GetMuzzleLocation(), WeaponMesh1PComp->GetMuzzleLocation()+FVector::OneVector, WeaponMesh1PComp->GetMuzzleRotation().Quaternion(), ECollisionChannel::ECC_Camera, FCollisionShape::MakeSphere(ProbeSize), QueryParams);
		if (bCollide)
		{
			OnWeaponCollide(Result);
		}
		else
		{
			FHitResult EmptyCollide(ForceInit);
			EmptyCollide.bBlockingHit = false;
			OnWeaponCollide(EmptyCollide);
		}
	}
}

void AINSWeaponBase::OnWeaponCollide(const FHitResult& CollideResult)
{
	if (CollideResult.bBlockingHit)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s collided with other things,collide actor:%s,collide component:%s"), *GetName(), *CollideResult.GetActor()->GetName(), *CollideResult.Component->GetName());
	}
	if (GetOwnerCharacter())
	{
		GetOwnerCharacter()->OnWeaponCollide(CollideResult);
	}
}

void AINSWeaponBase::SimulateEachSingleShoot()
{
	
}

void AINSWeaponBase::AddWeaponSpread(FVector& OutSpreadDir, FVector& BaseDirection)
{
	const int32 RandomSeed = FMath::Rand();
	const FRandomStream WeaponRandomStream(RandomSeed);
	const float RandomFloat = FMath::RandRange(0.5f, 1.5f);
	const float ConeHalfAngle = FMath::DegreesToRadians(WeaponSpreadData.CurrentWeaponSpread * RandomFloat);
	OutSpreadDir = WeaponRandomStream.VRandCone(BaseDirection, ConeHalfAngle, ConeHalfAngle);
}


void AINSWeaponBase::GetFireLoc(FVector& OutFireLoc)
{
	OutFireLoc = WeaponMesh1PComp->GetMuzzleLocation();
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
	if (HasAuthority() && CurrentClipAmmo > 0)
	{
		CurrentClipAmmo = FMath::Clamp<int32>(CurrentClipAmmo - 1, 0, CurrentClipAmmo);
		if (CurrentClipAmmo == 0)
		{
			StopWeaponFire();
			bDryReload = true;
			StartReloadWeapon();
		}
	}
}

void AINSWeaponBase::OnRep_WeaponFireCount()
{
	AINSPlayerCharacter* const PlayerCharacter = Cast<AINSPlayerCharacter>(GetOwnerCharacter());
	AINSPlayerController* const OwnerPC = GetOwnerPlayer<AINSPlayerController>();
	if (PlayerCharacter && !PlayerCharacter->GetIsDead())
	{
		if (OwnerPC)
		{
			PlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
			PlayerCharacter->Get1PAnimInstance()->PlayFireAnim();
			GetWeapon1PAnimInstance()->PlayFireAnim();
			OwnerPC->PlayerCameraManager->PlayCameraShake(FireCameraShakingClass);
		}
		else
		{
			PlayerCharacter->Get3PAnimInstance()->PlayFireAnim();
			GetWeapon3pAnimINstance()->PlayFireAnim();
		}
		SimulateWeaponFireFX();
	}
}

void AINSWeaponBase::OnRep_ScanTraceHit()
{

}

void AINSWeaponBase::OnRep_OwnerCharacter()
{
	if (!IsNetMode(NM_DedicatedServer))
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
	DOREPLIFETIME(AINSWeaponBase, CurrentWeaponFireMode);
	DOREPLIFETIME(AINSWeaponBase, RepWeaponFireCount);
	DOREPLIFETIME(AINSWeaponBase, OwnerCharacter);
	DOREPLIFETIME(AINSWeaponBase, bIsAimingWeapon);
	DOREPLIFETIME(AINSWeaponBase, bWantsToEquip);
	DOREPLIFETIME(AINSWeaponBase, ScanTraceHitLoc);
	DOREPLIFETIME_CONDITION(AINSWeaponBase, CurrentClipAmmo,COND_OwnerOnly);
	DOREPLIFETIME(AINSWeaponBase,CurrentWeaponBasePoseType);
	DOREPLIFETIME_CONDITION(AINSWeaponBase,WeaponConfigData, COND_InitialOnly);
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
	if (GetOwner())
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
	
}

void AINSWeaponBase::OnRep_Equipping()
{

}

void AINSWeaponBase::OnRep_CurrentClipAmmo()
{
	bDryReload = CurrentClipAmmo == 0;
}

void AINSWeaponBase::StartReloadWeapon()
{
	if (GetLocalRole() == ROLE_Authority && CheckCanReload())
	{
		if (GetOwnerCharacter()->GetIsAiming())
		{
			StopWeaponAim();
		}
		SetWeaponState(EWeaponState::RELOADIND);
		PlayWeaponReloadAnim();
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
		bIsAimingWeapon = true;
		GetOwnerCharacter()->SetCharacterAiming(bIsAimingWeapon);
		if (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			OnRep_AimWeapon();
		}
	}
}

void AINSWeaponBase::StopWeaponAim()
{
	bIsAimingWeapon = false;
	GetOwnerCharacter()->SetCharacterAiming(bIsAimingWeapon);
	if (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
	{
		OnRep_AimWeapon();
	}
}

bool AINSWeaponBase::CheckCanAim()
{
	AINSPlayerController* const OwnerPC = Cast<AINSPlayerController>(GetOwner());
	const AINSPlayerCharacter* const OwnerChar = OwnerPC == nullptr ? nullptr : OwnerPC->GetINSPlayerCharacter();
#if UE_SERVER
	if (!OwnerPC)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("Weapon :%s has no owner player , can't Aim"), *GetName());
		return false;
	}
	if (!OwnerChar)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("Weapon :%s has no owner character , can't Aim"), *GetName());
		return false;
	}
#endif
	return OwnerPC && OwnerChar;
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
		Cast<AINSPlayerController>(GetOwnerCharacter()->GetController())->AddPitchInput(RecoilAmount * DeltaTimeSeconds);
	}
}

void AINSWeaponBase::UpdateRecoilHorizontally(float DeltaTimeSeconds, float RecoilAmount)
{
	if (GetOwnerCharacter())
	{
		RecoilAmount = FMath::RandRange(-RecoilAmount, RecoilAmount);
		Cast<AINSPlayerController>(GetOwnerCharacter()->GetController())->AddYawInput(RecoilAmount * DeltaTimeSeconds);
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
			break;
		}
	}
}

void AINSWeaponBase::FinishSwitchFireMode()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetWeaponState(EWeaponState::IDLE);
	}
}
