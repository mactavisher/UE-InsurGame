// Fill out your copyright notice in the Description page of Project Settings.


#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "INSCharacter/INSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "INSAnimation/INSCharacterAimInstance.h"
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
#include "Camera/CameraShakeBase.h"
#include "INSCharacter/INSPlayerCameraManager.h"
#include "INSAnimation/INSWeaponAnimInstance.h"
#include "INSHud/INSHUDBase.h"
#include "INSComponents/INSCharacterAudioComponent.h"
#include "INSAssets/INSStaticAnimData.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "INSComponents/INSCharSkeletalMeshComponent.h"
#include "INSWeaponCrossHair/INSCrossHair_Cross.h"
#include "INSItems/INSWeaponAttachments/INSWeaponAttachment_Optic.h"
#ifndef UINSCrossHairBase
#include "INSWeaponCrossHair/INSCrossHairBase.h"
#endif
#ifndef UWorld
#include "Engine/World.h"
#endif // !UWorld

DEFINE_LOG_CATEGORY(LogINSWeapon);

AINSWeaponBase::AINSWeaponBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	ItemType = EItemType::WEAPON;
	AvailableFireModes.Add(EWeaponFireMode::FULLAUTO);
	AvailableFireModes.Add(EWeaponFireMode::SINGLE);
	CurrentWeaponFireMode = AvailableFireModes[0];
	CurrentClipAmmo = WeaponConfigData.AmmoPerClip;
	AmmoLeft = WeaponConfigData.MaxAmmo;
	CurrentWeaponState = EWeaponState::NONE;
	CurrentWeaponBasePoseType = EWeaponBasePoseType::DEFAULT;
	LastFireTime = 0.f;
	RepWeaponFireCount = 0;
	bIsAimingWeapon = false;
#if WITH_EDITORONLY_DATA
	bInfinityAmmo = false;
#endif
	AimTime = 0.3f;
	RecoilVerticallyFactor = -3.f;
	RecoilHorizontallyFactor = 6.f;
	bDryReload = false;
	SetReplicatingMovement(false);
	WeaponMeshComp = ObjectInitializer.CreateDefaultSubobject<UINSWeaponMeshComponent>(this, TEXT("WeaponMesh1PComp"));
	WeaponParticleComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticelSystemComp"));
	OpticRail = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this,TEXT("OpticRailComp"));
	OpticRail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OpticRail->SetupAttachment(RootComponent);
	WeaponMeshComp->AlwaysLoadOnClient = true;
	WeaponMeshComp->AlwaysLoadOnServer = true;
	WeaponMeshComp->SetReceivesDecals(false);
	RootComponent = WeaponMeshComp;
	WeaponParticleComp->SetupAttachment(RootComponent);
	WeaponMeshComp->SetHiddenInGame(false);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	bEnableAutoReload = true;
	CurrentWeaponZoomState = EZoomState::ZOMMEDOUT;
	WeaponType = EWeaponType::NONE;
#if WITH_EDITORONLY_DATA
	bShowDebugTrace = false;
#endif
	ProbeSize = 10.f;
	ZoomedInEventTriggered = false;
	ZoomedOutEventTriggered = false;
	CrossHairClass = UINSCrossHair_Cross::StaticClass();
	bSupressorEquiped = false;
	DefaultAimingFOV = 80.f;
	bUsingNoFrontSightMesh = false;
	BaseAimHandIKXLocation = 2.f;
	BaseHandsOffSetLoc = FVector(10.f, 0.f, 0.f);
	bIsFiring = false;
	SemiAutoCount = 0;
	bClientCosmeticWeapon = false;
}

void AINSWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateWeaponCollide();
	UpdateADSStatus(DeltaTime);
	UpdateWeaponSpread(DeltaTime);
	ClearFiring(DeltaTime);
}

void AINSWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (!WeaponConfigData.bRequireExtraOpticRail && OpticRail)
	{
		OpticRail->DestroyComponent();
		OpticRail = nullptr;
	}
}

void AINSWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	UpdateCharAnimationBasePoseType(CurrentWeaponBasePoseType);
	UpdateWeaponMeshVisibility();
	if (GetLocalRole() == ROLE_Authority)
	{
		ClientCreateWeaponCrossHair();
		OnRep_WeaponBasePoseType();
		CheckAndEquipWeaponAttachment();
	}
	//set up attach ment for optic rail
	if (OpticRail)
	{
	  OpticRail->AttachToComponent(WeaponMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale,TEXT("OpticRail"));
	}
	const FWeaponAttachmentSlot* SightSlot = GetWeaponAttachmentSlot(WeaponAttachmentSlotName::Sight);
	if (SightSlot && SightSlot->GetWeaponAttachmentInstance())
	{
		WeaponMeshComp->SetSkeletalMesh(WeaponMeshNoFrontSight);
	}
	else
	{
		WeaponMeshComp->SetSkeletalMesh(WeaponMeshWithFrontSight);
	}
	if(HasAuthority())
	{
		OnRep_MeshType();
	}
}

void AINSWeaponBase::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
}

void AINSWeaponBase::StartWeaponFire()
{
	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		if (CurrentWeaponFireMode == EWeaponFireMode::SINGLE)
		{
			FireShot();
		}
		else if (CurrentWeaponFireMode == EWeaponFireMode::SEMIAUTO)
		{
			GetWorld()->GetTimerManager().SetTimer(SemiAutoTimerHandle, this, &AINSWeaponBase::FireShot, GetTimeBetweenShots() * 0.8f, true, 0.f);
		}
		else if (CurrentWeaponFireMode == EWeaponFireMode::FULLAUTO)
		{
			GetWorld()->GetTimerManager().SetTimer(FullAutoTimerHandle, this, &AINSWeaponBase::FireShot, GetTimeBetweenShots(), true, 0.f);
		}
	}
}

void AINSWeaponBase::InspectWeapon()
{
}

bool AINSWeaponBase::CheckCanSwitchFireMode()
{
	AINSPlayerController* PlayerController = Cast<AINSPlayerController>(GetOwner());
	AINSPlayerCharacter* PlayerCharacter = PlayerController == nullptr ? nullptr : PlayerController->GetINSPlayerCharacter();
	if (!PlayerController)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no owner player , can't Switch Fire Mode"), *GetName());
		return false;
	}
	if (!PlayerCharacter)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no owner player character ,  can't Switch Fire Mode"), *GetName());
		return false;
	}
	if (PlayerCharacter->GetIsDead())
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s and it's owner charcter is dead,  can't Switch Fire Mode "), *GetName());
		return false;
	}
	if (CurrentWeaponState != EWeaponState::IDLE)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is busy,  can't Switch Fire Mode "), *GetName());
		return false;
	}
	if (AvailableFireModes.Num() == 1)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has only one fire mode,can't Switch Fire Mode "), *GetName());
		return false;
	}
	return true;
}

void AINSWeaponBase::WeaponGoToIdleState()
{
}

bool AINSWeaponBase::GetIsWeaponInIdleState()
{
	return CurrentWeaponState == EWeaponState::IDLE && !bIsFiring;
}

void AINSWeaponBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	InitWeaponAttachmentSlots();
	if (WeaponAnimationClass)
	{
		WeaponAnimation = NewObject<UINSStaticAnimData>(this, WeaponAnimationClass);
	}
}

void AINSWeaponBase::InitWeaponAttachmentSlots()
{
	WeaponAttachmentSlots.Add(WeaponAttachmentSlotName::Muzzle, FWeaponAttachmentSlot(EWeaponAttachmentType::MUZZLE, true));
	WeaponAttachmentSlots.Add(WeaponAttachmentSlotName::Sight, FWeaponAttachmentSlot(EWeaponAttachmentType::SIGHT, true));
	WeaponAttachmentSlots.Add(WeaponAttachmentSlotName::UnderBarrel, FWeaponAttachmentSlot(EWeaponAttachmentType::UNDER_BARREL, true));
	WeaponAttachmentSlots.Add(WeaponAttachmentSlotName::LeftRail, FWeaponAttachmentSlot(EWeaponAttachmentType::LEFT_RAIL, true));
	WeaponAttachmentSlots.Add(WeaponAttachmentSlotName::RightRail, FWeaponAttachmentSlot(EWeaponAttachmentType::RIGHT_RAIL, true));
}


void AINSWeaponBase::GetADSSightTransform(FTransform& OutTransform)
{
	if (WeaponMeshComp)
	{
		OutTransform = WeaponMeshComp->GetSocketTransform(TEXT("M68Carryhandle"));
	}
}

FWeaponAttachmentSlot* AINSWeaponBase::GetWeaponAttachmentSlot(FName SlotName)
{
	FWeaponAttachmentSlot* TargetAttachmentSlot = WeaponAttachmentSlots.Find(SlotName);
	if (!TargetAttachmentSlot)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("Weapon Slot Named %s Not found!"));
		return nullptr;
	}
	return TargetAttachmentSlot;
}

void AINSWeaponBase::FireProjectile(FVector_NetQuantize100 SpawnLoc, FVector_NetQuantize100 SpawnDir)
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
		world->LineTraceSingleByChannel(ScanTraceHitResult, SpawnLoc, SpawnLoc + SpawnDir * WeaponConfigData.ScanTraceRange, ECC_Camera, WeaponScanTraceParam);
		AINSProjectile* SpawnedProjectile = world->SpawnActorDeferred<AINSProjectile>(ProjectileClass, ProjectileSpawnTransform, GetOwnerCharacter()->GetController(), GetOwnerCharacter(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (SpawnedProjectile)
		{
			SpawnedProjectile->SetOwnerWeapon(this);
			SpawnedProjectile->SetIsFakeProjectile(false);
			SpawnedProjectile->SetMuzzleSpeed(WeaponConfigData.MuzzleSpeed);
			SpawnedProjectile->SetCurrentPenetrateCount(0);
			SpawnedProjectile->SetInstigatedPlayer(Cast<AController>(GetOwner()));
			SpawnedProjectile->SetScanTraceProjectile(ScanTraceHitResult.bBlockingHit);
			SpawnedProjectile->SetScanTraceHitLoc(ScanTraceHitResult.bBlockingHit ? FVector::ZeroVector : ScanTraceHitResult.ImpactPoint);
			SpawnedProjectile->SetScanTraceTime(WeaponConfigData.ScanTraceRange / WeaponConfigData.MuzzleSpeed);
			SpawnedProjectile->SetSpawnLocation(SpawnLoc);
			UGameplayStatics::FinishSpawningActor(SpawnedProjectile, ProjectileSpawnTransform);
			ConsumeAmmo();
			RepWeaponFireCount++;
			OnRep_WeaponFireCount();
		}
	}
}


void AINSWeaponBase::ServerFireProjectile_Implementation(FVector SpawnLoc, FVector SpawnDir)
{
	FireProjectile(SpawnLoc, SpawnDir);
}

bool AINSWeaponBase::ServerFireProjectile_Validate(FVector SpawnLoc, FVector SpawnDir)
{
	return true;
}

void AINSWeaponBase::SetOwnerCharacter(class AINSCharacter* NewOwnerCharacter)
{
	if (HasAuthority())
	{
		this->OwnerCharacter = NewOwnerCharacter;
		OnRep_OwnerCharacter();
	}
}

void AINSWeaponBase::SetWeaponState(const EWeaponState NewWeaponState)
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

void AINSWeaponBase::PostFireShot()
{
	if (CurrentWeaponFireMode == EWeaponFireMode::SEMIAUTO)
	{
		SemiAutoCount += static_cast<uint8>(1);
	}
	if (CurrentClipAmmo == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(SemiAutoTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(FullAutoTimerHandle);
	}
	if ((SemiAutoCount == 3))
	{
		GetWorld()->GetTimerManager().ClearTimer(SemiAutoTimerHandle);
		//reset the semi auto count
		SemiAutoCount = static_cast<uint8>(0);
	}
	GetWorld()->GetTimerManager().ClearTimer(SemiAutoTimerHandle);
}


void AINSWeaponBase::ServerSetWeaponState_Implementation(EWeaponState NewWeaponState)
{
	SetWeaponState(NewWeaponState);
}

bool AINSWeaponBase::ServerSetWeaponState_Validate(EWeaponState NewWeaponState)
{
	return true;
}

FORCEINLINE class UINSWeaponAnimInstance* AINSWeaponBase::GetWeaponAnimInstance()
{
	return Cast<UINSWeaponAnimInstance>(WeaponMeshComp->AnimScriptInstance);
}


FTransform AINSWeaponBase::GetSightsTransform()
{
	const uint8 NumAttachment = CachedWeaponAttachmentInstances.Num();
	if (NumAttachment > 0)
	{
		for (int i = 0; i < NumAttachment; i++)
		{
			AINSWeaponAttachment* AttachmentIns = CachedWeaponAttachmentInstances[i];
			if (AttachmentIns->GetCurrentAttachmentType() == EWeaponAttachmentType::SIGHT)
			{
				AINSWeaponAttachment_Optic* Optic = Cast<AINSWeaponAttachment_Optic>(AttachmentIns);
				if (Optic)
				{
					if (!Optic->GetOpticSightTransform().Equals(FTransform::Identity))
					{
						return Optic->GetOpticSightTransform();
					}
					break;
				}
			}
		}
	}
	return WeaponMeshComp->GetSocketTransform(SightAlignerSocketName, RTS_World);
}

bool AINSWeaponBase::CheckScanTraceRange()
{
	FVector TraceStart(ForceInit);
	FVector TraceDir(ForceInit);
	FVector TraceEnd(ForceInit);
	TraceDir = WeaponMeshComp->GetMuzzleForwardVector();
	TraceStart = WeaponMeshComp->GetMuzzleLocation();
	TraceEnd = TraceStart + TraceDir * WeaponConfigData.ScanTraceRange;
	FHitResult TraceHit(ForceInit);
	FCollisionQueryParams ScanTraceQuery;
	ScanTraceQuery.AddIgnoredActor(this);
	ScanTraceQuery.AddIgnoredActor(this->GetOwnerCharacter());
	ScanTraceQuery.bReturnPhysicalMaterial = false;
	GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Camera, ScanTraceQuery);
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	if (bShowDebugTrace)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 2.f);
		DrawDebugSphere(GetWorld(), TraceStart, 10.f, 8.f, FColor::Red, false, 2.f);
		DrawDebugSphere(GetWorld(), TraceEnd, 10.f, 8.f, FColor::Red, false, 2.f);
	}

#endif
	if (TraceHit.bBlockingHit)
	{
		return true;
	}
	return false;
}


void AINSWeaponBase::FireShot()
{
	if (!CheckCanFire())
	{
		return;
	}
	//get the base fire location
	FVector FireLoc(ForceInit);
	GetBarrelStartLoc(FireLoc);

	//get the fire shot dir
	FVector FireDir(ForceInit);
	GetFireDir(FireDir);

	//add weapon fire spread
	FVector SpreadDir(ForceInit);

	ApplyWeaponSpread(SpreadDir, FireDir);
	if (HasAuthority())
	{
		FireProjectile(FireLoc, SpreadDir);

#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
		if (bShowDebugTrace && GetOwnerCharacter()->IsLocallyControlled())
		{
			const FVector TraceStart = WeaponMeshComp->GetMuzzleLocation();
			const FVector TraceEnd = TraceStart + SpreadDir * 1000;
			DrawDebugLine(GetWorld(), WeaponMeshComp->GetMuzzleLocation(), TraceEnd, FColor::Blue, false, 2.f);
			DrawDebugSphere(GetWorld(), FireLoc, 5.f, 5, FColor::Red, false, 10.f);
		}
#endif
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerFireProjectile(FireLoc, SpreadDir);
	}

	bIsFiring = true;

	PostFireShot();
}


void AINSWeaponBase::OnZoomedOut()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	UGameplayStatics::SpawnSoundAttached(ADSOutSound, WeaponMeshComp);
}

void AINSWeaponBase::OnZoomedIn()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	UGameplayStatics::SpawnSoundAttached(ADSInSound, WeaponMeshComp);
}

void AINSWeaponBase::DrawCrossHair(class UCanvas* InCanvas, const FLinearColor DrawColor)
{
	if (CrossHair && ADSAlpha < 0.4f && CurrentWeaponState != EWeaponState::RELOADIND)
	{
		CrossHair->DrawCrossHair(InCanvas, this, FLinearColor::Red);
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

bool AINSWeaponBase::IsSightAlignerExist() const
{
	return WeaponMeshComp->DoesSocketExist(SightAlignerSocketName);
}


void AINSWeaponBase::ClientCreateWeaponCrossHair_Implementation()
{
	if (CrossHairClass)
	{
		CrossHair = NewObject<UINSCrossHairBase>(this, CrossHairClass);
		if (GetOwner())
		{
			CrossHair->SetOwnerPlayer(GetINSPlayerController());
			CrossHair->SetOwnerWeapon(this);
		}
	}
}

bool AINSWeaponBase::ClientCreateWeaponCrossHair_Validate()
{
	return true;
}

void AINSWeaponBase::GetWeaponAttachmentInstances(TArray<AINSWeaponAttachment*>& OutAttachmentInstance)
{
}

float AINSWeaponBase::GetWeaponAimHandIKXLocation()
{
	const uint8 NumAttachment = CachedWeaponAttachmentInstances.Num();
	if (NumAttachment > 0)
	{
		for (int i = 0; i < NumAttachment; i++)
		{
			const AINSWeaponAttachment* const AttachmentIns = CachedWeaponAttachmentInstances[i];
			if (AttachmentIns->GetCurrentAttachmentType() == EWeaponAttachmentType::SIGHT)
			{
				const AINSWeaponAttachment_Optic* Optic = Cast<AINSWeaponAttachment_Optic>(AttachmentIns);
				if (Optic)
				{
					return Optic->GetBaseHandsIKXValue();
				}
			}
		}
	}
	return BaseAimHandIKXLocation;
}

void AINSWeaponBase::InsertSingleAmmo()
{
	if (AmmoLeft <= 0)
	{
		return;
	}
	CurrentClipAmmo += 1;
	AmmoLeft--;
}

void AINSWeaponBase::SetIsFiring(bool bFiring)
{
	bIsFiring = bFiring;
	if (bIsFiring)
	{
		OwnerCharacter->OnOutBoredState();
	}
}

void AINSWeaponBase::ClearFiring(float DeltaSeconds)
{
	if (bIsFiring)
	{
		float FireStateThreshold = GetTimeBetweenShots() * 0.3f;
		if (FireStateThreshold > 0.2f)
		{
			FireStateThreshold = 0.2f;
		}
		if (GetWorld()->GetTimeSeconds() - LastFireTime >= FireStateThreshold)
		{
			if (HasAuthority())
			{
				SetWeaponState(EWeaponState::IDLE);
			}
			else if (GetLocalRole() == ROLE_AutonomousProxy)
			{
				ServerSetWeaponState(EWeaponState::IDLE);
			}
			bIsFiring = false;
		}
	}
}

void AINSWeaponBase::SetLocalClientCosmeticWeapon(AINSWeaponBase* InWeapon)
{
	this->LocalClientCosmeticWeapon = InWeapon;
	InWeapon->SetActorHiddenInGame(true);
	InWeapon->GetWeaponMeshComp()->bCastHiddenShadow = true;
	InWeapon->bClientCosmeticWeapon = true;
}


void AINSWeaponBase::ServerInsertSingleAmmo_Implementation()
{
	InsertSingleAmmo();
}

bool AINSWeaponBase::ServerInsertSingleAmmo_Validate()
{
	return true;
}

void AINSWeaponBase::OnRep_CurrentWeaponState()
{
	switch (CurrentWeaponState)
	{
	case EWeaponState::IDLE: break;
	case EWeaponState::RELOADIND: OnWeaponStartReload();
		break;
	case EWeaponState::UNEQUIPED: break;
	case EWeaponState::EQUIPPING: OnWeaponStartEquip();
		break;
	case EWeaponState::FIREMODESWITCHING: OnWeaponSwitchFireMode();
		break;
	case EWeaponState::EQUIPED: break;
	case EWeaponState::NONE: break;
	default: break;
	}
}

void AINSWeaponBase::OnRep_AimWeapon()
{
}

void AINSWeaponBase::OnRep_WeaponBasePoseType()
{
	FString Message;
	Message.Append("Weapon:");
	Message.Append(GetName());
	Message.Append("Base pose Type Replicated!Current Weapon base type is : ");
	switch (CurrentWeaponBasePoseType)
	{
	case EWeaponBasePoseType::ALTGRIP: Message.Append("Alt Grip");
		break;
	case EWeaponBasePoseType::FOREGRIP: Message.Append("Fore Grip");
		break;
	case EWeaponBasePoseType::DEFAULT: Message.Append("Default");
		break;
	default: break;
	}
#if WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, Message);
#endif
	UE_LOG(LogINSWeapon, Log, TEXT("%s"), *Message);
	UpdateCharAnimationBasePoseType(CurrentWeaponBasePoseType);
	if (WeaponMeshComp->AnimScriptInstance)
	{
		Cast<UINSWeaponAnimInstance>(WeaponMeshComp->AnimScriptInstance)->SetWeaponBasePoseType(CurrentWeaponBasePoseType);
	}
}

void AINSWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();
}

void AINSWeaponBase::Destroyed()
{
	Super::Destroyed();
	for (const TPair<FName, FWeaponAttachmentSlot>& pair : WeaponAttachmentSlots)
	{
		FWeaponAttachmentSlot& CurrentSlot = (FWeaponAttachmentSlot&)pair.Value;
		if (CurrentSlot.WeaponAttachmentInstance)
		{
			CurrentSlot.WeaponAttachmentInstance->Destroy();
		}
	}
	if(LocalClientCosmeticWeapon)
	{
		LocalClientCosmeticWeapon->Destroy();
	}
}

void AINSWeaponBase::SimulateWeaponFireFX()
{
	//spawn shooting sound
	USoundCue* SelectedFireSound = nullptr;
	UParticleSystem* SelectFireParticleTemplate = nullptr;
	UINSWeaponMeshComponent* SelectedFXAttachParent = WeaponMeshComp;
	if (IsNetMode(NM_DedicatedServer))
	{
		//UE_LOG(LogINSWeapon, Log, TEXT("Weapon firing but effects should not played in Dedicated server"));
		return;
	}
	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		SelectedFireSound = bSupressorEquiped ? SupressedFireSound1P : FireSound1P;
		SelectFireParticleTemplate = FireParticle1P;
	}
	else
	{
		SelectedFireSound = bSupressorEquiped ? SupressedFireSound3P : FireSound3P;
		SelectFireParticleTemplate = FireParticle3P;
	}

	//spawn muzzle emitter
	if (SelectFireParticleTemplate)
	{
		WeaponParticleComp = UGameplayStatics::SpawnEmitterAttached(SelectFireParticleTemplate, SelectedFXAttachParent, SelectedFXAttachParent->GetWeaponSockets().MuzzleFlashSocket);
	}

	if (SelectedFireSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SelectedFireSound, SelectedFXAttachParent->GetMuzzleLocation(), SelectedFXAttachParent->GetMuzzleRotation());
	}

	//cast projectile shell
	if (!ProjectileShellClass || !GetOwnerCharacter())
	{
		return;
	}
	const FTransform ShellSpawnTran = WeaponMeshComp->GetShellSpawnTransform();
	AINSProjectileShell* ShellActor = GetWorld()->SpawnActorDeferred<AINSProjectileShell>(ProjectileShellClass, ShellSpawnTran, this, GetOwnerCharacter());
	if (ShellActor)
	{
		UGameplayStatics::FinishSpawningActor(ShellActor, ShellSpawnTran);
	}
}

void AINSWeaponBase::FindCrossHairHit(FHitResult& Hit)
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
		const FVector TraceDir = ViewRot.Vector();
		//sightly move forward  the trace start location to avoid hit selves
		const FVector TraceStart = ViewLoc + TraceDir * 100.f;
		const float TraceRange = 30000.f;
		const FVector TraceEnd = TraceStart + TraceDir * TraceRange;
		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(this);
		queryParams.AddIgnoredActor(this->GetOwnerCharacter());
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Camera, queryParams);
		if (Hit.bBlockingHit && Hit.GetActor())
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

void AINSWeaponBase::CheckAndEquipWeaponAttachment()
{
	if (HasAuthority())
	{
		for (const TPair<FName, FWeaponAttachmentSlot>& pair : WeaponAttachmentSlots)
		{
			FWeaponAttachmentSlot& CurrentSlot = (FWeaponAttachmentSlot&)pair.Value;
			if (CurrentSlot.bIsAvailable && CurrentSlot.WeaponAttachmentClass)
			{
				CurrentSlot.WeaponAttachmentInstance = GetWorld()->SpawnActorDeferred<AINSWeaponAttachment>(CurrentSlot.GetWeaponAttachmentClass()
				                                                                                            , GetActorTransform()
				                                                                                            , this
				                                                                                            , GetOwnerCharacter()
				                                                                                            , ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
				CurrentSlot.GetWeaponAttachmentInstance()->SetWeaponOwner(this);
				CurrentSlot.GetWeaponAttachmentInstance()->SetOwner(GetOwner());
				UGameplayStatics::FinishSpawningActor(CurrentSlot.GetWeaponAttachmentInstance(), GetActorTransform());
				AddAttachmentInstance(CurrentSlot.GetWeaponAttachmentInstance());
				if (CurrentSlot.GetWeaponAttachmentInstance()->GetCurrentAttachmentType() == EWeaponAttachmentType::SIGHT)
				{
					bUsingNoFrontSightMesh = true;
				}
			}
		}
	}
}

void AINSWeaponBase::AddAttachmentInstance(class AINSWeaponAttachment* AttachmentToAdd)
{
	if (AttachmentToAdd)
	{
		CachedWeaponAttachmentInstances.Add(AttachmentToAdd);
	}
}

float AINSWeaponBase::GetAimFOV()
{
	const uint8 NumAttachment = CachedWeaponAttachmentInstances.Num();
	if (NumAttachment > 0)
	{
		for (int i = 0; i < NumAttachment; i++)
		{
			const AINSWeaponAttachment* const AttachmentIns = CachedWeaponAttachmentInstances[i];
			if (AttachmentIns->GetCurrentAttachmentType() == EWeaponAttachmentType::SIGHT)
			{
				return AttachmentIns->GetTargetFOV();
			}
		}
	}
	return DefaultAimingFOV;
}

void AINSWeaponBase::GetFireDir(FVector& OutDir)
{
	if (!GetOwner())
	{
		OutDir = FVector::ZeroVector;
	}
	else
	{
		FHitResult CrossHairScanTraceHit(ForceInit);
		FindCrossHairHit(CrossHairScanTraceHit);
		if (CrossHairScanTraceHit.bBlockingHit)
		{
			OutDir = CrossHairScanTraceHit.Location - WeaponMeshComp->GetMuzzleLocation();
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
	AINSPlayerCharacter* PlayerCharacter = PlayerController == nullptr ? nullptr : PlayerController->GetINSPlayerCharacter();
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
	if (CurrentClipAmmo == 0)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s current clip is empty, can't fire "), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::RELOADIND)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is reloading,can't fire "), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::FIREMODESWITCHING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is switching fire mode,can't fire "), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::EQUIPPING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is equiping,can't fire "), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::UNEQUIPING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is un-equiping,can't fire "), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::UNEQUIPED)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is un-equiped,can't fire "), *GetName());
		return false;
	}
	if ((GetWorld()->GetTimeSeconds() - LastFireTime) < WeaponConfigData.TimeBetweenShots)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s fire interval not allowd,can't fire"), *GetName());
		return false;
	}
	if (bIsFiring)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is in fire state,can't fire"), *GetName());
		return false;
	}
	return true;
}

void AINSWeaponBase::StopWeaponFire()
{
	GetWorldTimerManager().ClearTimer(SemiAutoTimerHandle);
	GetWorldTimerManager().ClearTimer(FullAutoTimerHandle);
}

void AINSWeaponBase::ServerStopWeaponFire_Implementation()
{
	StopWeaponFire();
}

bool AINSWeaponBase::ServerStopWeaponFire_Validate()
{
	return true;
}

bool AINSWeaponBase::CheckCanReload()
{
	AINSPlayerController* const PlayerController = GetOwnerPlayer<AINSPlayerController>();
	if (!PlayerController)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no owner player , can't reload"), *GetName());
		return false;
	}
	const AINSPlayerCharacter* const PlayerCharacter = PlayerController == nullptr ? nullptr : PlayerController->GetINSPlayerCharacter();
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
	if (CurrentClipAmmo == WeaponConfigData.AmmoPerClip)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s current clip is full, No Need reloading"), *GetName());
		return false;
	}
	if (AmmoLeft <= 0)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s has no ammo left, cant't reload"), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::RELOADIND)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is Already reloading,can't Reload"), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::FIREMODESWITCHING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is switching fire mode,can't Reload "), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::EQUIPPING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is equiping,can't Reload "), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::UNEQUIPING)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is un-equiping,can't Reload "), *GetName());
		return false;
	}
	if (CurrentWeaponState == EWeaponState::UNEQUIPED)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s is un-equiped,can't Reload "), *GetName());
		return false;
	}

	return true;
}

void AINSWeaponBase::OnWeaponStartReload()
{
	AINSPlayerCharacter* const OwnerPlayerCharacter = GetOwnerCharacter<AINSPlayerCharacter>();
	if (OwnerPlayerCharacter && !IsNetMode(NM_DedicatedServer))
	{
		if (GetLocalRole() >= ROLE_AutonomousProxy)
		{
			OwnerPlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
			OwnerPlayerCharacter->Get1PAnimInstance()->PlayReloadAnim(bDryReload);
			GetWeaponAnimInstance()->PlayReloadAnim(bDryReload);
			if(LocalClientCosmeticWeapon)
			{
				LocalClientCosmeticWeapon->OnWeaponStartReload();
			}
		}
		else
		{
			OwnerPlayerCharacter->Get3PAnimInstance()->PlayReloadAnim(bDryReload);
			GetWeaponAnimInstance()->PlayReloadAnim(bDryReload);
			UINSCharacterAudioComponent* const CharacterAudioComp = GetOwnerCharacter()->GetCharacterAudioComp();
			if (CharacterAudioComp)
			{
				CharacterAudioComp->OnWeaponReload();
			}
		}
	}
}

void AINSWeaponBase::OnWeaponSwitchFireMode()
{
	AINSPlayerCharacter* const OwnerPlayerCharacter = GetOwnerCharacter<AINSPlayerCharacter>();
	if (OwnerPlayerCharacter && !IsNetMode(NM_DedicatedServer))
	{
		
			OwnerPlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
			OwnerPlayerCharacter->Get1PAnimInstance()->PlaySwitchFireModeAnim();
			GetWeaponAnimInstance()->PlaySwitchFireModeAnim();
	}
}

void AINSWeaponBase::OnWeaponStartEquip()
{
	AINSPlayerCharacter* const OwnerPlayerCharacter = GetOwnerCharacter<AINSPlayerCharacter>();
	if (OwnerPlayerCharacter && !IsNetMode(NM_DedicatedServer))
	{
		if (GetLocalRole() >= ROLE_AutonomousProxy)
		{
			OwnerPlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
			OwnerPlayerCharacter->Get1PAnimInstance()->PlayWeaponStartEquipAnim();
			GetWeaponAnimInstance()->PlayWeaponStartEquipAnim();
			if(LocalClientCosmeticWeapon)
			{
				LocalClientCosmeticWeapon->OnWeaponStartEquip();
			}
		}
		else
		{
			OwnerPlayerCharacter->Get3PAnimInstance()->PlayWeaponStartEquipAnim();
			GetWeaponAnimInstance()->PlayWeaponStartEquipAnim();
		}
	}
}

void AINSWeaponBase::UpdateWeaponCollide()
{
	if (GetOwnerCharacter() && !GetOwnerCharacter()->GetIsDead() && GetLocalRole() == ROLE_AutonomousProxy)
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(this->GetOwner());
		QueryParams.AddIgnoredComponent(WeaponMeshComp);
		QueryParams.AddIgnoredActor(this->GetOwnerCharacter());
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		QueryParams.bDebugQuery = true;
#endif
		FHitResult Result;
		const bool bCollide = GetWorld()->SweepSingleByChannel(Result, WeaponMeshComp->GetMuzzleLocation(),
		                                                       WeaponMeshComp->GetMuzzleLocation() + FVector::OneVector,
		                                                       WeaponMeshComp->GetMuzzleRotation().Quaternion(),
		                                                       ECC_Camera,
		                                                       FCollisionShape::MakeSphere(ProbeSize), QueryParams);
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
		const AActor* const CollidedActor = CollideResult.Actor.Get();
		const UActorComponent* CollidedComp = CollideResult.Component.Get();
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s collided with other things,collide actor:%s,collide component:%s")
		       , *GetName(), IsValid(CollidedActor)?*CollidedActor->GetName():TEXT("None"), IsValid(CollidedComp)?*CollidedComp->GetName():TEXT("None"));
	}
	if (GetOwnerCharacter())
	{
		GetOwnerCharacter()->OnWeaponCollide(CollideResult);
	}
}

void AINSWeaponBase::SimulateEachSingleShoot()
{
}

void AINSWeaponBase::ApplyWeaponSpread(FVector& OutSpreadDir, const FVector& BaseDirection)
{
	const int32 RandomSeed = FMath::Rand();
	const FRandomStream WeaponRandomStream(RandomSeed);
	const float RandomFloat = FMath::RandRange(0.5f, 1.5f);
	const float ConeHalfAngle = FMath::DegreesToRadians(WeaponSpreadData.CurrentWeaponSpread * RandomFloat);
	OutSpreadDir = WeaponRandomStream.VRandCone(BaseDirection, ConeHalfAngle, ConeHalfAngle);
}


void AINSWeaponBase::GetBarrelStartLoc(FVector& OutBarrelStartLoc)
{
	OutBarrelStartLoc = WeaponMeshComp->GetMuzzleLocation();
	FWeaponAttachmentSlot* WeaponAttachmentSlot = GetWeaponAttachmentSlot(WeaponAttachmentSlotName::Muzzle);
	if (WeaponAttachmentSlot && WeaponAttachmentSlot->GetWeaponAttachmentInstance())
	{
		OutBarrelStartLoc = WeaponAttachmentSlot->GetWeaponAttachmentInstance()->GetBarrelLocation();
	}
}

void AINSWeaponBase::CalculateAmmoAfterReload()
{
#if WITH_EDITORONLY_DATA
	if (bInfinityAmmo)
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("this Weapon :%s infinit ammo mode has enabled,reloading will consumes no ammo "), *GetName());
		CurrentClipAmmo = WeaponConfigData.AmmoPerClip;
		return;
	}
#endif
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


void AINSWeaponBase::ConsumeAmmo()
{
#if WITH_EDITORONLY_DATA
	if (bInfinityAmmo)
	{
		return;
	}
#endif
	if (HasAuthority() && CurrentClipAmmo > 0)
	{
		CurrentClipAmmo = FMath::Clamp<int32>(CurrentClipAmmo - 1, 0, CurrentClipAmmo);
	}
	if (CurrentClipAmmo == 0)
	{
		StopWeaponFire();
		bDryReload = true;
		if (bEnableAutoReload)
		{
			HandleWeaponReloadRequest();
		}
	}
}

void AINSWeaponBase::OnRep_WeaponFireCount()
{
	bIsFiring = true;
	if (!IsNetMode(NM_DedicatedServer))
	{
		AINSPlayerCharacter* const PlayerCharacter = Cast<AINSPlayerCharacter>(GetOwnerCharacter());
		AINSPlayerController* const OwnerPC = GetOwnerPlayer<AINSPlayerController>();
		if (PlayerCharacter && !PlayerCharacter->GetIsDead())
		{
			if (OwnerPC)
			{
				PlayerCharacter->Get1PAnimInstance()->StopFPPlayingWeaponIdleAnim();
				PlayerCharacter->Get1PAnimInstance()->PlayFireAnim();
				GetWeaponAnimInstance()->PlayFireAnim();
				OwnerPC->PlayerCameraManager->StartCameraShake(FireCameraShakingClass);
			}
			else
			{
				PlayerCharacter->Get3PAnimInstance()->PlayFireAnim();
				GetWeaponAnimInstance()->PlayFireAnim();
			}
			SimulateWeaponFireFX();
		}
		WeaponSpreadData.CurrentWeaponSpread = FMath::Clamp<float>(WeaponSpreadData.CurrentWeaponSpread + WeaponSpreadData.SpreadIncrementByShot, WeaponSpreadData.CurrentWeaponSpread, WeaponSpreadData.WeaponSpreadMax);
	}
}

void AINSWeaponBase::OnRep_ScanTraceHit()
{
}

void AINSWeaponBase::OnRep_OwnerCharacter()
{
	AINSPlayerCharacter* PlayerCharacter = Cast<AINSPlayerCharacter>(GetOwnerCharacter());
	if (!IsNetMode(NM_DedicatedServer))
	{
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
	// if (PlayerCharacter)
	// {
	// 	if (PlayerCharacter->GetLocalRole() >= ROLE_AutonomousProxy && GetINSPlayerController())
	// 	{
	// 		GetWeaponMeshComp()->AttachToComponent(PlayerCharacter->GetCharacter1PMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	// 	}
	// 	else
	// 	{
	// 		GetWeaponMeshComp()->AttachToComponent(PlayerCharacter->GetCharacter3PMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	// 	}
	// }
}

void AINSWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSWeaponBase, CurrentWeaponState);
	DOREPLIFETIME(AINSWeaponBase, CurrentWeaponFireMode);
	DOREPLIFETIME(AINSWeaponBase, RepWeaponFireCount);
	DOREPLIFETIME(AINSWeaponBase, OwnerCharacter);
	DOREPLIFETIME(AINSWeaponBase, bUsingNoFrontSightMesh);
	DOREPLIFETIME(AINSWeaponBase, bIsAimingWeapon);
	DOREPLIFETIME_CONDITION(AINSWeaponBase, CurrentClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME(AINSWeaponBase, CurrentWeaponBasePoseType);
	DOREPLIFETIME_CONDITION(AINSWeaponBase, WeaponConfigData, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AINSWeaponBase, AmmoLeft, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AINSWeaponBase, WeaponType, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AINSWeaponBase, bDryReload, COND_SimulatedOnly);
}


void AINSWeaponBase::OwnerPlayCameraShake()
{
	if (GetOwnerPlayer<AINSPlayerController>())
	{
		GetOwnerPlayer<AINSPlayerController>()->ClientStartCameraShake(FireCameraShakingClass);
	}
}


void AINSWeaponBase::UpdateWeaponSpread(float DeltaTimeSeconds)
{
	if (GetLocalRole() >= ROLE_AutonomousProxy && !IsNetMode(NM_DedicatedServer))
	{
		if (bIsAimingWeapon)
		{
			WeaponSpreadData.CurrentWeaponSpread = WeaponSpreadData.CurrentWeaponSpread * (1.f - ADSAlpha);
			if (ADSAlpha == 1.f)
			{
				WeaponSpreadData.CurrentWeaponSpread = 0.f;
			}
		}
		else
		{
			if (GetOwnerCharacter() && GetOwnerCharacter()->GetIsCharacterMoving())
			{
				WeaponSpreadData.CurrentWeaponSpread = FMath::Clamp<float>(
					WeaponSpreadData.CurrentWeaponSpread + DeltaTimeSeconds * WeaponSpreadData.
					MovementSpreadScalingFactor * WeaponSpreadData.SpreadIncrementByShot,
					WeaponSpreadData.CurrentWeaponSpread, WeaponSpreadData.WeaponSpreadMax);
			}
			else
			{
				if (!bIsFiring)
				{
					WeaponSpreadData.CurrentWeaponSpread = FMath::Clamp<float>(
						WeaponSpreadData.CurrentWeaponSpread - WeaponSpreadData.SpreadDecrement,
						WeaponSpreadData.DefaultWeaponSpread, WeaponSpreadData.CurrentWeaponSpread);
				}
			}
		}
	}
}

void AINSWeaponBase::UpdateCharAnimationBasePoseType(EWeaponBasePoseType NewType)
{
	if (GetOwnerCharacter())
	{
		GetOwnerCharacter()->SetWeaponBasePoseType(NewType);
	}
}

void AINSWeaponBase::OnRep_CurrentFireMode()
{
}

void AINSWeaponBase::OnRep_WeaponType()
{
}

void AINSWeaponBase::OnRep_Equipping()
{
}

void AINSWeaponBase::OnRep_MeshType()
{
	if (bUsingNoFrontSightMesh)
	{
		WeaponMeshComp->SetSkeletalMesh(WeaponMeshNoFrontSight);
	}
	else
	{
		WeaponMeshComp->SetSkeletalMesh(WeaponMeshWithFrontSight);
	}
}

void AINSWeaponBase::OnRep_CurrentClipAmmo()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		bDryReload = CurrentClipAmmo == 0;
	}
}

void AINSWeaponBase::HandleWeaponReloadRequest()
{
	if (HasAuthority())
	{
		if (CheckCanReload())
		{
			if (bIsAimingWeapon)
			{
				StopWeaponAim();
			}
			SetWeaponState(EWeaponState::RELOADIND);
		}
	}
}

void AINSWeaponBase::ServerHandleWeaponReloadRequest_Implementation()
{
	HandleWeaponReloadRequest();
}

bool AINSWeaponBase::ServerHandleWeaponReloadRequest_Validate()
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


void AINSWeaponBase::UpdateWeaponMeshVisibility()
{
	// if (GetLocalRole() >= ROLE_AutonomousProxy && GetOwnerCharacter() && GetOwnerCharacter()->GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	// {
	// 	WeaponMesh1PComp->SetHiddenInGame(false);
	// 	WeaponMesh3PComp->SetHiddenInGame(true);
	// 	WeaponMesh1PComp->SetComponentTickEnabled(true);
	// 	WeaponMesh3PComp->SetComponentTickEnabled(false);
	// 	WeaponMesh1PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	// 	WeaponMesh3PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	// 	WeaponMesh1PComp->SetCastShadow(true);
	// 	WeaponMesh1PComp->bCastDynamicShadow = true;
	// 	WeaponMesh3PComp->SetCastShadow(false);
	// 	WeaponMesh3PComp->bCastDynamicShadow = false;
	// }
	// else
	// {
	// 	WeaponMesh1PComp->SetHiddenInGame(true);
	// 	WeaponMesh3PComp->SetHiddenInGame(false);
	// 	WeaponMesh1PComp->SetComponentTickEnabled(false);
	// 	WeaponMesh3PComp->SetComponentTickEnabled(true);
	// 	WeaponMesh1PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	// 	WeaponMesh3PComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	// 	WeaponMesh1PComp->SetCastShadow(false);
	// 	WeaponMesh1PComp->bCastDynamicShadow = false;
	//
	// 	WeaponMesh3PComp->SetCastShadow(true);
	// 	WeaponMesh3PComp->bCastDynamicShadow = true;
	// }
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
		if (GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone)
		{
			OnRep_AimWeapon();
		}
	}
}

void AINSWeaponBase::StopWeaponAim()
{
	bIsAimingWeapon = false;
	GetOwnerCharacter()->SetCharacterAiming(bIsAimingWeapon);
	if (GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone)
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


class AINSPlayerController* AINSWeaponBase::GetINSPlayerController()
{
	if (GetOwner())
	{
		UE_LOG(LogINSWeapon, Warning, TEXT("Weapon :%s has No Owner Player"));
		return nullptr;
	}
	return Cast<AINSPlayerController>(GetOwner());
}


FString AINSWeaponBase::GetWeaponReadableCurrentState()
{
	FString ReadableCurrentWeaponState;
	switch (CurrentWeaponState)
	{
	case EWeaponState::IDLE: ReadableCurrentWeaponState.Append("Idle");
		break;
	case EWeaponState::RELOADIND: ReadableCurrentWeaponState.Append("reloading");
		break;
	case EWeaponState::UNEQUIPED: ReadableCurrentWeaponState.Append("unEquipped");
		break;
	case EWeaponState::EQUIPPING: ReadableCurrentWeaponState.Append("Equipping");
		break;
	case EWeaponState::FIREMODESWITCHING: ReadableCurrentWeaponState.Append("fire mode switching");
		break;
	case EWeaponState::UNEQUIPING: ReadableCurrentWeaponState.Append("unEquipping");
		break;
	case EWeaponState::EQUIPED: ReadableCurrentWeaponState.Append("equipped");
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
	if (CheckCanSwitchFireMode())
	{
		SetWeaponState(EWeaponState::FIREMODESWITCHING);
	}
}

void AINSWeaponBase::FinishSwitchFireMode()
{
	if (HasAuthority())
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
				if (NextFireModeIndex > AvailableFireModesNum - 1)
				{
					NextFireModeIndex = 0;
				}
				CurrentWeaponFireMode = AvailableFireModes[NextFireModeIndex];
				if (GetNetMode() == NM_Standalone || GetNetMode() == NM_ListenServer)
				{
					OnRep_CurrentFireMode();
				}
				break;
			}
		}
	}
	else
	{
		ServerFinishSwitchFireMode();
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
