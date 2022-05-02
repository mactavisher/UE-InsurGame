// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "INSCharacter/INSPlayerController.h"
#include "INSComponents/INSCharSkeletalMeshComponent.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSComponents/INSCharacterAudioComponent.h"
#include "INSGameModes/INSGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "DrawDebugHelpers.h"
#include "INSHud/INSHUDBase.h"
#include "Components/CapsuleComponent.h"
#include "INSCharacter/INSPlayerCameraManager.h"
#include "INSComponents/INSInventoryComponent.h"
#include "INSCore/INSGameInstance.h"
#include "Kismet/KismetMathLibrary.h"
#ifndef GEngine
#include "Engine/Engine.h"
#endif
#include "INSAnimation/INSFPAnimInstance.h"
#include "INSAnimation/INSTPAnimInstance.h"

AINSPlayerCharacter::AINSPlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharSkeletalMeshComponent>(MeshComponentName))
{
	bReplicates = true;
	SetReplicatingMovement(true);
	GetCapsuleComponent()->SetCapsuleHalfHeight(86.f);
	GetCapsuleComponent()->SetCapsuleRadius(40.f);
	BaseEyeHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.2f;
	CrouchedEyeHeight = BaseEyeHeight * 0.6f;
	CurrentEyeHeight = BaseEyeHeight;
	RootComponent = GetCapsuleComponent();
	FirstPersonCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	CharacterMesh3P = Cast<UINSCharSkeletalMeshComponent>(GetMesh());
	CharacterMesh3P->AddRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
	CharacterMesh1P = ObjectInitializer.CreateDefaultSubobject<UINSCharSkeletalMeshComponent>(this, TEXT("Character1pMeshComp"));
	InventoryComp = ObjectInitializer.CreateDefaultSubobject<UINSInventoryComponent>(this, TEXT("InventoryComp"));
	CharacterMesh1P->SetupAttachment(GetCapsuleComponent());
	CharacterMesh1P->AddRelativeLocation(FVector(0.f, -8.f, 0.f));
	CharacterMesh1P->SetAbsolute(false, false, true);
	CharacterMesh1P->bCastDynamicShadow = false;
	CharacterMesh1P->CastShadow = false;
	CharacterMesh1P->bReceivesDecals = false;
	CharacterMesh1P->LightingChannels.bChannel1 = true;
	CharacterMesh3P->SetupAttachment(RootComponent);
	CharacterMesh3P->AlwaysLoadOnClient = true;
	CharacterMesh3P->AlwaysLoadOnServer = true;
	CharacterMesh1P->AlwaysLoadOnClient = true;
	CharacterMesh1P->AlwaysLoadOnServer = true;
	CharacterMesh3P->bReceivesDecals = false;
	CharacterMesh3P->bLightAttachmentsAsGroup = true;
	CharacterMesh3P->LightingChannels.bChannel1 = true;
	CharacterMesh3P->bCastCapsuleIndirectShadow = true;
	CharacterMesh1P_Foot = ObjectInitializer.CreateDefaultSubobject<UINSCharSkeletalMeshComponent>(this,TEXT("CharacterMesh_Foot"));
	CharacterMesh1P_Foot->SetupAttachment(CharacterMesh3P);
	CharacterMesh1P_Foot->AddRelativeLocation(FVector(20.f, 0.f, 0.f));
	CharacterMesh1P_Foot->SetCastShadow(false);
	CharacterMesh1P_Foot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterMesh1P_Foot->SetCollisionResponseToAllChannels(ECR_Ignore);
	//CharacterMesh1P->AddRelativeLocation(FVector(0.f, 0.f, -174.f));
	CharacterMesh1P_Foot->SetHiddenInGame(false);
	bIsDead = false;
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	bShowDebugTrace = false;
#endif
}

void AINSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateComponents();
}

void AINSPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SetupAnimInstance();
	FirstPersonCamera->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale,TEXT("Bip01_CameraBoneSocket"));
	PhysicalAnimationComponent->SetSkeletalMeshComponent(CharacterMesh3P);
	if (CharacterAudioComp)
	{
		CharacterAudioComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,TEXT("Bip01_HeadSocket"));
		CharacterAudioComp->SetOwnerCharacter(this);
	}
}

void AINSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickRecoil(DeltaTime);
	UpdateCharacterMesh1P(DeltaTime);
}

void AINSPlayerCharacter::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (&FirstEquipTickFunction == &ThisTickFunction)
	{
		if (HasAuthority())
		{
			if (GetOwner() && InventoryComp->GetInitialized())
			{
				EquipBestWeapon();
				FirstEquipTickFunction.Target = nullptr;
				FirstEquipTickFunction.bCanEverTick = false;
				FirstEquipTickFunction.SetTickFunctionEnable(false);
				FirstEquipTickFunction.UnRegisterTickFunction();
				UE_LOG(LogINSCharacter, Log, TEXT("Character:%s has finished first equip tick ,unregistring"), *GetName());
			}
		}
	}
}

void AINSPlayerCharacter::OnCauseDamage(const FTakeHitInfo& HitInfo)
{
	Super::OnCauseDamage(HitInfo);
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	if (GetCharacterAudioComp())
	{
		GetCharacterAudioComp()->OnCauseDamage(HitInfo.bIsTeamDamage, HitInfo.bVictimDead);
	}
	if (LastHitInfo.Victim == LastHitInfo.InstigatorPawn)
	{
		return;
	}
	AINSPlayerCharacter* const PlayerCharacter = Cast<AINSPlayerCharacter>(LastHitInfo.InstigatorPawn);
	if (PlayerCharacter && !PlayerCharacter->GetIsDead() && !GetValid(PlayerCharacter))
	{
		if (PlayerCharacter->GetINSPlayerController())
		{
			PlayerCharacter->GetINSPlayerController()->PlayerCauseDamage(LastHitInfo);
		}
	}
}

void AINSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	INSPlayerController = Cast<AINSPlayerController>(GetController());
	if (GetNetMode() == NM_Standalone || GetNetMode() == NM_ListenServer)
	{
		OnRep_TeamType();
	}
	RegisterFirstEquipCheck();
}

void AINSPlayerCharacter::HandleMoveForwardRequest(float Value)
{
	Super::HandleMoveForwardRequest(Value);
}

void AINSPlayerCharacter::HandleMoveRightRequest(float Value)
{
	Super::HandleMoveRightRequest(Value);
}

void AINSPlayerCharacter::HandleStopSprintRequest()
{
	Super::HandleStopSprintRequest();
}

void AINSPlayerCharacter::HandleStartSprintRequest()
{
	Super::HandleStartSprintRequest();
}

void AINSPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSPlayerCharacter, MyTeamType);
}

void AINSPlayerCharacter::OnRep_CurrentWeapon()
{
	Super::OnRep_CurrentWeapon();
	if (CurrentWeapon)
	{
		for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
		{
			CachedAnimInstance->SetCurrentWeapon(CurrentWeapon);
			CachedAnimInstance->SetCurrentWeaponAnimData(CurrentWeapon->GetWeaponAnimDataPtr());
			CachedAnimInstance->SetBaseHandsIkLocation(CurrentWeapon->GetWeaponBaseIKLocation());
			CachedAnimInstance->SetAimHandIKXLocation(CurrentWeapon->GetWeaponAimHandIKXLocation());
		}
		if (GetLocalRole() >= ROLE_AutonomousProxy && !IsNetMode(NM_DedicatedServer))
		{
			UClass* CurrentWeaponClass = CurrentWeapon->GetClass();
			AINSWeaponBase* CosmeticWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(
				CurrentWeaponClass, CurrentWeapon->GetActorTransform(), CurrentWeapon->GetOwner()
				, CurrentWeapon->GetOwnerCharacter(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (CosmeticWeapon)
			{
				CosmeticWeapon->GetWeaponMeshComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				CosmeticWeapon->GetWeaponMeshComp()->SetCollisionResponseToAllChannels(ECR_Ignore);
				CosmeticWeapon->GetWeaponMeshComp()->AttachToComponent(CharacterMesh3P,
				                                                       FAttachmentTransformRules::SnapToTargetIncludingScale,
				                                                       TEXT("Bip01_Weapon1Socket"));
				CurrentWeapon->SetLocalClientCosmeticWeapon(CosmeticWeapon);
			}
		}
		CurrentWeapon->GetWeaponMeshComp()->SetCastShadow(false);
		CurrentWeapon->GetWeaponMeshComp()->SetCastHiddenShadow(false);
		CurrentWeapon->GetWeaponMeshComp()->bCastDynamicShadow = false;
	}
	else
	{
		for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
		{
			if (CachedAnimInstance)
			{
				CachedAnimInstance->SetCurrentWeapon(nullptr);
				CachedAnimInstance->SetBaseHandsIkLocation(FVector::ZeroVector);
				CachedAnimInstance->SetAimHandIKXLocation(0.f);
				CachedAnimInstance->SetCurrentWeaponAnimData(nullptr);
			}
		}
	}
	if (HasAuthority() || GetLocalRole() == ROLE_AutonomousProxy)
	{
		AINSPlayerCameraManager* const CameraManager = Cast<AINSPlayerCameraManager>(GetINSPlayerController()->PlayerCameraManager);
		if (CameraManager)
		{
			CameraManager->SetCurrentWeapon(CurrentWeapon);
		}
	}
}


void AINSPlayerCharacter::SetTeamType(const ETeamType NewTeamType)
{
	MyTeamType = NewTeamType;
}

void AINSPlayerCharacter::OnEnterIdleState()
{
	Super::OnEnterIdleState();
	if (!GetCurrentWeapon())
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character:%s with no weapon is not moving for some time ,enter idle state"),
		       *GetName());
		for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
		{
			if (CachedAnimInstance)
			{
				CachedAnimInstance->SetIdleState(true);
			}
		}
	}
	if (GetCurrentWeapon() && GetCurrentWeapon()->GetCurrentWeaponState() == EWeaponState::IDLE)
	{
		UE_LOG(LogINSCharacter, Log,
		       TEXT("Character:%s with weapon %s is not moving and not using weapon for some time ,enter idle state"),
		       *GetName(), *(GetCurrentWeapon()->GetName()));
		for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
		{
			if (CachedAnimInstance)
			{
				CachedAnimInstance->SetIdleState(true);
			}
		}
	}
}

void AINSPlayerCharacter::RecalculateBaseEyeHeight()
{
	//Super::RecalculateBaseEyeHeight();
}

void AINSPlayerCharacter::OnEnterBoredState()
{
	Super::OnEnterBoredState();
	if (!GetCurrentWeapon())
	{
		UE_LOG(LogINSCharacter, Log,
		       TEXT("Character:%s with no weapon is not moving for too much time ,enter bored state"), *GetName());
		for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
		{
			if (CachedAnimInstance)
			{
				CachedAnimInstance->SetBoredState(true);
			}
		}
	}
	if (GetCurrentWeapon() && GetCurrentWeapon()->GetCurrentWeaponState() == EWeaponState::IDLE)
	{
		UE_LOG(LogINSCharacter, Log,
		       TEXT("Character:%s with weapon %s is not moving and not using weapon for too much time ,enter bored state"), *GetName(), *(GetCurrentWeapon()->GetName()));

		for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
		{
			if (CachedAnimInstance)
			{
				CachedAnimInstance->SetBoredState(true);
			}
		}
	}
}


void AINSPlayerCharacter::OnOutBoredState()
{
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			CachedAnimInstance->SetBoredState(false);
		}
	}
}


void AINSPlayerCharacter::OnLowHealth()
{
	Super::OnLowHealth();
	if (!IsNetMode(NM_DedicatedServer))
	{
		// TODO,do some audio effects
	}
}

void AINSPlayerCharacter::TickRecoil(float DeltaSeconds)
{
	if (CurrentWeapon && CurrentWeapon->GetIsFiring() && !bIsDead && INSPlayerController)
	{
		float YawRecoilAmount = FMath::RandRange(-CurrentWeapon->GetRecoilHorizontallyFactor(),
		                                         CurrentWeapon->GetRecoilHorizontallyFactor());
		AddControllerYawInput(YawRecoilAmount * DeltaSeconds);
		float PitchRecoilAmount = CurrentWeapon->GetRecoilVerticallyFactor();
		AddControllerPitchInput(PitchRecoilAmount * DeltaSeconds);
	}
}

void AINSPlayerCharacter::EquipFromInventory(const uint8 SlotIndex)
{
}

void AINSPlayerCharacter::UnEquipItem()
{
	Super::UnEquipItem();
}

void AINSPlayerCharacter::ServerUnEquipItem()
{
	Super::ServerUnEquipItem();
}

void AINSPlayerCharacter::FinishUnEquipItem()
{
	InventoryComp->PutItemInSlot(CurrentWeapon);
	Super::FinishUnEquipItem();
	if (PendingWeaponEquipEvent.bIsEventActive)
	{
		CreateAndEquipItem(PendingWeaponEquipEvent.ItemId, PendingWeaponEquipEvent.WeaponSlotIndex);
	}
	PendingWeaponEquipEvent.ResetEvent();
}

void AINSPlayerCharacter::Die()
{
	Super::Die();
	//the dead player pawn will have no controller,so need a null check here
	if (GetINSPlayerController())
	{
		GetINSPlayerController()->OnCharacterDead();
	}
}

void AINSPlayerCharacter::OnRep_Dead()
{
	Super::OnRep_Dead();
	if (bIsDead)
	{
		if (HasAuthority())
		{
			if (IsNetMode(NM_DedicatedServer))
			{
				CharacterMesh1P->SetHiddenInGame(true);
				CharacterMesh3P->SetHiddenInGame(true);
			}
			else
			{
				CharacterMesh1P->SetHiddenInGame(true);
				CharacterMesh3P->SetHiddenInGame(false);
				CharacterMesh3P->SetSimulatePhysics(true);
			}
		}
		else
		{
			if (CharacterMesh1P)
			{
				CharacterMesh1P->SetHiddenInGame(true);
			}
			CharacterMesh3P->SetHiddenInGame(false);
			CharacterMesh3P->SetAllBodiesBelowSimulatePhysics(LastHitInfo.HitBoneName, true);
			const float ShotDirPitchDecompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirPitch);
			const float ShotDirYawDeCompressed = FRotator::DecompressAxisFromByte(LastHitInfo.ShotDirYaw);
			const FRotator BloodSpawnRotation = FRotator(ShotDirPitchDecompressed, ShotDirYawDeCompressed, 0.f);
			CharacterMesh3P->AddImpulseToAllBodiesBelow(BloodSpawnRotation.Vector() * 1000.f, LastHitInfo.HitBoneName);
			CharacterMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			CharacterMesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
			//CharacterMesh3P->SetSimulatePhysics(true);
		}
	}
}

void AINSPlayerCharacter::SetWeaponBasePoseType(const EWeaponBasePoseType NewType)
{
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			CachedAnimInstance->SetWeaponBasePoseType(NewType);
		}
	}
}

UINSFPAnimInstance* AINSPlayerCharacter::GetFPAnimInstance()
{
	return FPSAnimInstance == nullptr ? Cast<UINSFPAnimInstance>(CharacterMesh1P->AnimScriptInstance) : FPSAnimInstance;
}

UINSTPAnimInstance* AINSPlayerCharacter::GetTPSAnimInstance()
{
	return TPSAnimInstance == nullptr ? Cast<UINSTPAnimInstance>(CharacterMesh3P->AnimScriptInstance) : TPSAnimInstance;
}

void AINSPlayerCharacter::OnShotFired()
{
	Super::OnShotFired();
	if (InventoryComp && CurrentWeapon)
	{
		FInventorySlot* InventorySlot = InventoryComp->GetItemSlot(CurrentWeapon->GetInventorySlotIndex());
		if (InventorySlot)
		{
			InventorySlot->ClipAmmo = CurrentWeapon->GetCurrentClipAmmo();
		}
	}
}

void AINSPlayerCharacter::OnReloadFinished()
{
	Super::OnReloadFinished();
	if (InventoryComp && CurrentWeapon)
	{
		FInventorySlot* InventorySlot = InventoryComp->GetItemSlot(CurrentWeapon->GetInventorySlotIndex());
		if (InventorySlot)
		{
			InventorySlot->ClipAmmo = CurrentWeapon->GetAmmoLeft();
		}
	}
}

void AINSPlayerCharacter::GetCacheCharAnimInstances(TArray<UINSCharacterAimInstance*>& OutAnimInstances)
{
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			OutAnimInstances.Emplace(CachedAnimInstance);
		}
	}
}

float AINSPlayerCharacter::PlayWeaponUnEquipAnim()
{
	float AnimDuration = 0.f;
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		AnimDuration = CachedAnimInstance->PlayWeaponUnEquipAnim();
	}
	return AnimDuration;
}

float AINSPlayerCharacter::PlayWeaponEquipAnim()
{
	float AnimDuration = 0.f;
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		AnimDuration = CachedAnimInstance->PlayWeaponEquipAnim();
	}
	return AnimDuration;
}

float AINSPlayerCharacter::PlayFireAnim()
{
	float Duration = 0.f;
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			CachedAnimInstance->StopFPPlayMoveAnimation();
			Duration = CachedAnimInstance->PlayFireAnim();
		}
	}
	return Duration;
}

float AINSPlayerCharacter::PlayWeaponReloadAnim()
{
	float Duration = 0.f;
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			Duration = CachedAnimInstance->PlayWeaponReloadAnim(CurrentWeapon->GetIsDryReload());
		}
	}
	return Duration;
}

float AINSPlayerCharacter::PlayWeaponSwitchFireModeAnim()
{
	float Duration = 0.f;
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			CachedAnimInstance->StopFPPlayMoveAnimation();
			Duration = CachedAnimInstance->PlayWeaponSwitchFireModeAnim();
		}
	}
	return Duration;
}

void AINSPlayerCharacter::ReceiveInventoryInitialized()
{
	Super::ReceiveInventoryInitialized();
}

void AINSPlayerCharacter::OnOutIdleState()
{
	Super::OnOutIdleState();
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			CachedAnimInstance->SetIdleState(false);
		}
	}
}


void AINSPlayerCharacter::OnRep_Aim()
{
	Super::OnRep_Aim();
	if (!CurrentWeapon || GetNetMode() == NM_DedicatedServer || !GetINSPlayerController())
	{
		return;
	}
	AINSPlayerCameraManager* CameraManager = Cast<AINSPlayerCameraManager>(GetINSPlayerController()->PlayerCameraManager);
	if (!CameraManager)
	{
		return;
	}
	CameraManager->SetAimingFOV(CurrentWeapon->GetAimFOV());
	bIsAiming
		? CameraManager->OnAim(CurrentWeapon->GetWeaponAimTime())
		: CameraManager->OnStopAim(CurrentWeapon->GetWeaponAimTime());
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			CachedAnimInstance->SetIsAiming(bIsAiming);
		}
	}
}

void AINSPlayerCharacter::OnRep_IsCrouched()
{
	Super::OnRep_IsCrouched();
	if (GetFPAnimInstance())
	{
		GetFPAnimInstance()->SetIsCrouching(bIsCrouched);
	}
	if (GetTPSAnimInstance())
	{
		GetTPSAnimInstance()->SetCurrentStance(ECharacterStance::CROUCH);
		GetTPSAnimInstance()->SetIsCrouching(bIsCrouched);
	}
}

void AINSPlayerCharacter::OnRep_Sprint()
{
	Super::OnRep_Sprint();
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			CachedAnimInstance->SetSprintPressed(bIsSprint);
		}
	}
}

void AINSPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (HasAuthority())
	{
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
		FString DebugMessage;
		DebugMessage.Append(TEXT("Charcter:"));
		DebugMessage.Append(GetName());
		DebugMessage.Append("'s PlayerState Replicated!");
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, DebugMessage);
#endif
	}
}

void AINSPlayerCharacter::OnRep_LastHitInfo()
{
	Super::OnRep_LastHitInfo();
	OnCauseDamage(LastHitInfo);
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	if (LastHitInfo.Victim == this)
	{
		FString DebugMessage;
		DebugMessage.Append("you are taking damage, damage token: ").Append(FString::FromInt(LastHitInfo.Damage));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, DebugMessage);
	}
#endif
}

void AINSPlayerCharacter::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);
}

void AINSPlayerCharacter::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);
}

void AINSPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AINSPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AINSPlayerCharacter::SetAimHandsXLocation(const float Value)
{
	if (GetFPAnimInstance())
	{
		GetFPAnimInstance()->SetAimHandIKXLocation(Value);
	}
}

void AINSPlayerCharacter::UpdateComponents()
{
	if (FirstPersonCamera)
	{
		if (IsNetMode(NM_DedicatedServer))
		{
			FirstPersonCamera->DestroyComponent(false);
			//FirstPersonCamera->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			FirstPersonCamera = nullptr;
		}
	}
	if (CharacterMesh1P)
	{
		CharacterMesh1P->SetOnlyOwnerSee(true);
		if (IsNetMode(NM_DedicatedServer) || GetLocalRole() < ROLE_AutonomousProxy)
		{
			CharacterMesh1P->DestroyComponent(false);
			//CharacterMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			CharacterMesh1P = nullptr;
		}
	}
	if (CharacterMesh1P_Foot)
	{
		if (IsNetMode(NM_DedicatedServer) || GetLocalRole() < ROLE_AutonomousProxy)
		{
			CharacterMesh1P_Foot->DestroyComponent(false);
			//CharacterMesh1P_Foot->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			CharacterMesh1P_Foot = nullptr;
		}
		else
		{
			CharacterMesh1P_Foot->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale);
			CharacterMesh1P_Foot->SetMasterPoseComponent(GetCharacter3PMesh());
		}
	}
	if (CharacterMesh3P)
	{
		CharacterMesh3P->SetOwnerNoSee(true);
		// if (GetLocalRole()==ROLE_AutonomousProxy)
		// {
		// 	CharacterMesh3P->DestroyComponent(false);
		// 	//CharacterMesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		// 	CharacterMesh3P = nullptr;
		// }
	}
}

void AINSPlayerCharacter::RegisterFirstEquipCheck()
{
	if (HasAuthority())
	{
		FirstEquipTickFunction.Target = this;
		FirstEquipTickFunction.bCanEverTick = true;
		FirstEquipTickFunction.TickInterval = 0.1f;
		FirstEquipTickFunction.SetTickFunctionEnable(true);
		FirstEquipTickFunction.RegisterTickFunction(GetLevel());
	}
}

bool AINSPlayerCharacter::CheckCharacterIsReady()
{
	if (GetFPAnimInstance() && GetFPAnimInstance()->GetIsAnimInitialized() && GetFPAnimInstance()->GetIsValidPlayAnim())
	{
		return true;
	}
	return false;
}

void AINSPlayerCharacter::HandleCrouchRequest()
{
	Super::HandleCrouchRequest();
}

void AINSPlayerCharacter::HandleItemEquipRequest(const int32 NextItemId, const uint8 SlotIndex)
{
	Super::HandleItemEquipRequest(NextItemId, SlotIndex);
	if (HasAuthority())
	{
		EquipItem(NextItemId, SlotIndex);
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerEquipItem(NextItemId, SlotIndex);
	}
}

void AINSPlayerCharacter::HandleItemFinishUnEquipRequest()
{
	Super::HandleItemFinishUnEquipRequest();
}

void AINSPlayerCharacter::ServerEquipItem_Implementation(const int32 NextItemId, const uint8 SlotIndex)
{
	EquipItem(NextItemId, SlotIndex);
}

bool AINSPlayerCharacter::ServerEquipItem_Validate(const int32 NextItemId, const uint8 SlotIndex)
{
	return true;
}

void AINSPlayerCharacter::EquipItem(int32 NextItemId, uint8 NextSlotIndex)
{
	const FInventorySlot* SelectedSlot = InventoryComp->GetItemSlot(NextSlotIndex);
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetInventorySlotIndex() != NextSlotIndex)
		{
			UnEquipItem();
			SetupPendingWeaponEquipEvent(SelectedSlot->ItemId, NextSlotIndex);
		}
	}
	else
	{
		const int32 SelectedItemId = SelectedSlot == nullptr ? NextItemId : SelectedSlot->ItemId;
		CreateAndEquipItem(SelectedItemId, NextSlotIndex);
	}
}

void AINSPlayerCharacter::SetCurrentAnimData(UINSStaticAnimData* AnimData)
{
	Super::SetCurrentAnimData(AnimData);
	for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
	{
		if (CachedAnimInstance)
		{
			CachedAnimInstance->SetCurrentWeaponAnimData(CurrentAnimPtr);
		}
	}
}

void AINSPlayerCharacter::EquipGameModeDefaultWeapon()
{
	UClass* CurrentWeaponClass = GetINSPlayerController()->GetGameModeRandomWeapon();
	if (!CurrentWeaponClass)
	{
		return;
	}
	class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(
		CurrentWeaponClass, GetActorTransform(), GetOwner(), this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (NewWeapon)
	{
		NewWeapon->SetAutonomousProxy(true);
		NewWeapon->SetWeaponState(EWeaponState::NONE);
		NewWeapon->SetOwner(GetOwner());
		NewWeapon->SetOwnerCharacter(this);
		UGameplayStatics::FinishSpawningActor(NewWeapon, GetActorTransform());
	}
	SetCurrentWeapon(NewWeapon);
}

void AINSPlayerCharacter::EquipBestWeapon()
{
	if (InventoryComp)
	{
		const uint8 ItemSlotIdx = InventoryComp->GiveBestWeapon();
		AINSItems* ItemInstance = InventoryComp->GetItemFromInventory(0, ItemSlotIdx);
		if (ItemInstance && ItemInstance->GetClass()->IsChildOf(AINSWeaponBase::StaticClass()))
		{
			SetCurrentWeapon(Cast<AINSWeaponBase>(ItemInstance));
		}
	}
}

void AINSPlayerCharacter::ServerEquipBestWeapon_Implementation()
{
	EquipBestWeapon();
}

bool AINSPlayerCharacter::ServerEquipBestWeapon_Validate()
{
	return true;
}

void AINSPlayerCharacter::OnRep_TeamType()
{
	if (MyTeamType == ETeamType::ALLIE)
	{
		CharacterMesh1P->SetSkeletalMesh(CTDefaultMesh.Mesh1p);
		CharacterMesh3P->SetSkeletalMesh(CTDefaultMesh.Mesh3p);
	}
	if (MyTeamType == ETeamType::REBEL)
	{
		CharacterMesh1P->SetSkeletalMesh(TerroristDefaultMesh.Mesh1p);
		CharacterMesh3P->SetSkeletalMesh(TerroristDefaultMesh.Mesh3p);
	}
}


void AINSPlayerCharacter::UpdateCharacterMesh1P(float DeltaTime)
{
	if (!GetIsDead() && GetLocalRole() >= ROLE_AutonomousProxy && !IsNetMode(NM_DedicatedServer))
	{
		CharacterMesh1P->SetRelativeLocation(FVector(0.f, 0.f, -CurrentEyeHeight));
		if (bIsCrouched)
		{
			//CurrentEyeHeight = FMath::FInterpTo(CurrentEyeHeight,CrouchedEyeHeight,DeltaTime,1.f);
			CurrentEyeHeight = FMath::Clamp<float>(CurrentEyeHeight - 0.01f, CrouchedEyeHeight, CurrentEyeHeight);
		}
		else
		{
			//CurrentEyeHeight = FMath::FInterpTo(CurrentEyeHeight,BaseEyeHeight,DeltaTime,1.f);
			CurrentEyeHeight = FMath::Clamp<float>(CurrentEyeHeight + 0.01f, CrouchedEyeHeight, BaseEyeHeight);
		}
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, FString::SanitizeFloat(CurrentEyeHeight));
		CharacterMesh1P->SetWorldRotation(GetControlRotation());
		FTransform Mesh1pPelvisTrans = CharacterMesh1P->GetSocketTransform(TEXT("Bip01_SpineSocket"));
		FTransform CapsuleTransFrom = GetCapsuleComponent()->GetComponentTransform();
		FRotator ControlRot = GetControlRotation();
		float ClampedAngle = FMath::ClampAngle(ControlRot.Pitch, -90.f, 90.f);
		float PitchOffSetMultiplier = FMath::Abs(ClampedAngle / 90.f);
		CapsuleTransFrom.SetLocation(CapsuleTransFrom.GetLocation() + FVector(
			-15.f * PitchOffSetMultiplier, 0.f, CurrentEyeHeight * PitchOffSetMultiplier * 0.35f));
		FTransform RelativeTransForm = UKismetMathLibrary::MakeRelativeTransform(Mesh1pPelvisTrans, CapsuleTransFrom);
		FVector RelLocation = RelativeTransForm.GetLocation();
		RelLocation.Y = 0.f;
		CharacterMesh1P->AddRelativeLocation(-RelLocation);
	}
}

void AINSPlayerCharacter::SetupAnimInstance()
{
	FPSAnimInstance = Cast<UINSFPAnimInstance>(CharacterMesh1P->AnimScriptInstance);
	CachedAnimInstances.Emplace(FPSAnimInstance);
	TPSAnimInstance = Cast<UINSTPAnimInstance>(CharacterMesh3P->AnimScriptInstance);
	CachedAnimInstances.Emplace(TPSAnimInstance);
}

void AINSPlayerCharacter::UpdateAnimationData(class AINSItems* InItemRef)
{
	const EItemType ItemType = InItemRef->GetItemType();
	if (ItemType == EItemType::WEAPON)
	{
		AINSWeaponBase* WeaponItem = Cast<AINSWeaponBase>(InItemRef);
		if (WeaponItem)
		{
			for (UINSCharacterAimInstance* CachedAnimInstance : CachedAnimInstances)
			{
				CachedAnimInstance->SetCurrentWeaponAnimData(WeaponItem->GetWeaponAnimDataPtr());
			}
		}
	}
}

void AINSPlayerCharacter::OnRep_Owner()
{
	Super::OnRep_Owner();
	INSPlayerController = Cast<AINSPlayerController>(GetOwner());
	if (!IsNetMode(NM_DedicatedServer))
	{
		const bool bMyController = INSPlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (bMyController && CharacterAudioComp)
		{
			CharacterAudioComp->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale,TEXT("Bip01_HeadSocket"));
			CharacterAudioComp->SetOwnerCharacter(this);
		}
	}
}

void AINSPlayerCharacter::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	INSPlayerController = Cast<AINSPlayerController>(NewOwner);
}

void AINSPlayerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
}

void AINSPlayerCharacter::UpdateCrouchEyeHeightSmoothly()
{
}

void AINSPlayerCharacter::ReceiveFriendlyFire(class AINSPlayerController* InstigatorPlayer, float DamageTaken)
{
}

FTransform AINSPlayerCharacter::GetPlayerCameraTransform() const
{
	return FirstPersonCamera->GetRelativeTransform();
}

void AINSPlayerCharacter::GetPlayerCameraSocketWorldTransform(FTransform& OutCameraSocketTransform)
{
	if (GetLocalRole() == ROLE_Authority || GetLocalRole() == ROLE_AutonomousProxy)
	{
		OutCameraSocketTransform = FirstPersonCamera->GetComponentTransform();
	}
	else
	{
		OutCameraSocketTransform = FTransform::Identity;
	}
}


void AINSPlayerCharacter::SetCurrentWeapon(class AINSWeaponBase* NewWeapon)
{
	Super::SetCurrentWeapon(NewWeapon);
	// if (HasAuthority() && InventoryComp)
	// {
	// 	InventoryComp->InitItemData(NewWeapon);
	// }
}

void AINSPlayerCharacter::ReceiveSetupWeaponAttachment()
{
	UINSCharSkeletalMeshComponent* ItemAttachParent = nullptr;
	const ENetMode NetMode = GetNetMode();
	if (NetMode == NM_DedicatedServer)
	{
		ItemAttachParent = GetCharacter1PMesh();
	}
	if (NetMode == NM_Standalone)
	{
		ItemAttachParent = GetWorld()->GetFirstPlayerController() == GetINSPlayerController() ? GetCharacter1PMesh() : GetCharacter3PMesh();
	}
	if (NetMode == NM_Client)
	{
		ItemAttachParent = GetLocalRole() >= ROLE_AutonomousProxy ? GetCharacter1PMesh() : GetCharacter3PMesh();
	}
	if (NetMode == NM_ListenServer)
	{
		ItemAttachParent = GetWorld()->GetFirstPlayerController() == GetINSPlayerController() ? GetCharacter1PMesh() : GetCharacter3PMesh();
	}
	if (ItemAttachParent)
	{
		CurrentWeapon->AttachToComponent(ItemAttachParent, FAttachmentTransformRules::SnapToTargetIncludingScale,TEXT("Bip01_Weapon1Socket"));
	}
}
