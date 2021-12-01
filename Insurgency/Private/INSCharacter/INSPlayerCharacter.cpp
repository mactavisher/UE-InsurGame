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
#include "INSAnimation/INSCharacterAimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "INSCharacter/INSPlayerStateBase.h"
#include "DrawDebugHelpers.h"
#include "INSHud/INSHUDBase.h"
#include "Components/CapsuleComponent.h"
#include "INSAnimation/INSFPAnimInstance.h"
#include "INSAnimation/INSTPAnimInstance.h"
#include "INSCharacter/INSPlayerCameraManager.h"
#include "INSComponents/INSInventoryComponent.h"
#include "Kismet/KismetMathLibrary.h"
#ifndef GEngine
#include "Engine/Engine.h"
#endif

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
	CharacterMesh1P_Foot->SetHiddenInGame(true);
	bIsDead = false;
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	bShowDebugTrace = false;
#endif
}

void AINSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetupMeshVisibility();
	if (GetLocalRole() >= ROLE_AutonomousProxy && !IsNetMode(NM_DedicatedServer))
	{
		CharacterReadyTick.bCanEverTick = true;
		CharacterReadyTick.TickInterval = 0.1f;
		CharacterReadyTick.SetTickFunctionEnable(true);
		CharacterReadyTick.Target = this;
		CharacterReadyTick.RegisterTickFunction(GetLevel());
		CharacterMesh1P_Foot->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale);
		CharacterMesh1P_Foot->SetMasterPoseComponent(GetCharacter3PMesh());
	}
	if (GetLocalRole() < ROLE_AutonomousProxy)
	{
		CharacterMesh1P->DestroyComponent();
		FirstPersonCamera->DestroyComponent();
		CharacterMesh1P_Foot->DestroyComponent();
		UE_LOG(LogINSCharacter, Log, TEXT("Componens unnesessary for simulated clients get destoryed "));
	}
}

void AINSPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	FirstPersonCamera->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_CameraBoneSocket"));
	PhysicalAnimationComponent->SetSkeletalMeshComponent(CharacterMesh3P);
	if (CharacterAudioComp)
	{
		CharacterAudioComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,TEXT("Bip01_HeadSocket"));
		CharacterAudioComp->SetOwnerCharacter(this);
	}
	//PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow()
}

void AINSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickRecoil(DeltaTime);
	UpdateCharacterMesh1P(DeltaTime);
	//CheckPendingEquipWeapon(DeltaTime);
}

void AINSPlayerCharacter::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (&ThisTickFunction == &CharacterReadyTick)
	{
		if (CheckCharacterIsReady())
		{
			if (CurrentWeapon)
			{
				if (HasAuthority())
				{
					CurrentWeapon->SetWeaponState(EWeaponState::EQUIPPING);
				}
				else
				{
					CurrentWeapon->ServerSetWeaponState(EWeaponState::EQUIPPING);
				}
				if (Get1PAnimInstance())
				{
					Get1PAnimInstance()->SetBaseHandsIkLocation(CurrentWeapon->GetWeaponBaseIKLocation());
					Get1PAnimInstance()->SetAimHandIKXLocation(CurrentWeapon->GetWeaponAimHandIKXLocation());
				}
			}
			CharacterReadyTick.bCanEverTick = false;
			CharacterReadyTick.SetTickFunctionEnable(false);
			CharacterReadyTick.UnRegisterTickFunction();
			UE_LOG(LogINSCharacter, Log, TEXT("Character:%s is ready for action,and check ready tick function is unregistered"), *GetName());
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
	if (PlayerCharacter && !PlayerCharacter->GetIsDead() && !PlayerCharacter->IsPendingKill())
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
	SetupMeshVisibility();
	if (GetNetMode() == NM_Standalone || GetNetMode() == NM_ListenServer)
	{
		OnRep_TeamType();
	}
	if (HasAuthority())
	{
		//EquipGameModeDefaultWeapon();
		EquipBestWeapon();
	}
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
		SetupWeaponAttachment();
		if (Get1PAnimInstance())
		{
			Get1PAnimInstance()->SetCurrentWeapon(CurrentWeapon);
			SetCurrentAnimData(CurrentWeapon->GetWeaponAnimDataPtr());
			Get1PAnimInstance()->SetBaseHandsIkLocation(CurrentWeapon->GetWeaponBaseIKLocation());
			Get1PAnimInstance()->SetAimHandIKXLocation(CurrentWeapon->GetWeaponAimHandIKXLocation());
		}
		Get3PAnimInstance()->SetBaseHandsIkLocation(CurrentWeapon->GetWeaponBaseIKLocation());
		Get3PAnimInstance()->SetCurrentWeapon(CurrentWeapon);
		if (GetLocalRole() >= ROLE_AutonomousProxy && !IsNetMode(NM_DedicatedServer))
		{
			UClass* CurrentWeaponClass = CurrentWeapon->GetClass();
			AINSWeaponBase* CosmeticWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(CurrentWeaponClass, CurrentWeapon->GetActorTransform(), CurrentWeapon->GetOwner()
			                                                                                , CurrentWeapon->GetOwnerCharacter(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (CosmeticWeapon)
			{
				CosmeticWeapon->GetWeaponMeshComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				CosmeticWeapon->GetWeaponMeshComp()->SetCollisionResponseToAllChannels(ECR_Ignore);
				CosmeticWeapon->GetWeaponMeshComp()->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale,TEXT("Bip01_Weapon1Socket"));
				CurrentWeapon->SetLocalClientCosmeticWeapon(CosmeticWeapon);
			}
		}
	}
	else
	{
		if (Get1PAnimInstance())
		{
			Get1PAnimInstance()->SetCurrentWeapon(nullptr);
			Get1PAnimInstance()->SetBaseHandsIkLocation(FVector::ZeroVector);
			Get1PAnimInstance()->SetAimHandIKXLocation(0.f);
		}
		Get3PAnimInstance()->SetCurrentWeapon(nullptr);
		SetCurrentAnimData(nullptr);

		Get3PAnimInstance()->SetBaseHandsIkLocation(FVector::ZeroVector);
	}
	CharacterReadyTick.bCanEverTick = true;
	CharacterReadyTick.SetTickFunctionEnable(true);
	if (HasAuthority() || GetLocalRole() == ROLE_AutonomousProxy)
	{
		AINSPlayerCameraManager* const CameraManager = Cast<AINSPlayerCameraManager>(GetINSPlayerController()->PlayerCameraManager);
		if (CameraManager)
		{
			CameraManager->SetCurrentWeapon(CurrentWeapon);
		}
		CurrentWeapon->GetWeaponMeshComp()->SetCastShadow(false);
		CurrentWeapon->GetWeaponMeshComp()->SetCastHiddenShadow(false);
		CurrentWeapon->GetWeaponMeshComp()->bCastDynamicShadow = false;
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
		UE_LOG(LogINSCharacter, Log, TEXT("Character:%s with no weapon is not moving for some time ,enter idle state"), *GetName());
		if (Get1PAnimInstance())
		{
			Get1PAnimInstance()->SetIdleState(true);
		}
		Get3PAnimInstance()->SetIdleState(true);
	}
	if (GetCurrentWeapon() && GetCurrentWeapon()->GetCurrentWeaponState() == EWeaponState::IDLE)
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character:%s with weapon %s is not moving and not using weapon for some time ,enter idle state"), *GetName(), *(GetCurrentWeapon()->GetName()));
		if (Get1PAnimInstance())
		{
			Get1PAnimInstance()->SetIdleState(true);
		}
		Get3PAnimInstance()->SetIdleState(true);
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
		UE_LOG(LogINSCharacter, Log, TEXT("Character:%s with no weapon is not moving for too much time ,enter bored state"), *GetName());
		if (Get1PAnimInstance())
		{
			Get1PAnimInstance()->SetBoredState(true);
		}
		Get3PAnimInstance()->SetBoredState(true);
	}
	if (GetCurrentWeapon() && GetCurrentWeapon()->GetCurrentWeaponState() == EWeaponState::IDLE)
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character:%s with weapon %s is not moving and not using weapon for too much time ,enter bored state"), *GetName(), *(GetCurrentWeapon()->GetName()));

		if (Get1PAnimInstance())
		{
			Get1PAnimInstance()->SetBoredState(true);
		}
		Get3PAnimInstance()->SetBoredState(true);
	}
}


void AINSPlayerCharacter::OnOutBoredState()
{
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetBoredState(false);
	}
	Get3PAnimInstance()->SetBoredState(false);
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
		float YawRecoilAmount = FMath::RandRange(-CurrentWeapon->GetRecoilHorizontallyFactor(), CurrentWeapon->GetRecoilHorizontallyFactor());
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
	Super::FinishUnEquipItem();
	FInventorySlot* SelectedSlot = InventoryComp->GetItemSlot(CurrentWeapon->GetInventorySlotIndex());
	if(SelectedSlot)
	{
		SelectedSlot->AmmoLeft = CurrentWeapon->AmmoLeft;
		SelectedSlot->ClipAmmo = CurrentWeapon->GetCurrentClipAmmo();
	}
	CurrentWeapon->Destroy();
	CurrentWeapon=nullptr;
	if(PendingWeaponEquipEvent.bIsEventActive)
	{
		class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(PendingWeaponEquipEvent.WeaponClass, GetActorTransform(), GetOwner(), this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (NewWeapon)
		{
			NewWeapon->SetAutonomousProxy(true);
			NewWeapon->SetOwner(GetOwner());
			NewWeapon->SetOwnerCharacter(this);
			NewWeapon->SetInventorySlotIndex(PendingWeaponEquipEvent.WeaponSlotIndex);
			UGameplayStatics::FinishSpawningActor(NewWeapon, GetActorTransform());
		}
		SetCurrentWeapon(NewWeapon);
		NewWeapon->SetWeaponState(EWeaponState::EQUIPPING);
		PendingWeaponEquipEvent.bIsEventActive = false;
	}
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
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetWeaponBasePoseType(NewType);
	}
	Get3PAnimInstance()->SetWeaponBasePoseType(NewType);
}

void AINSPlayerCharacter::OnOutIdleState()
{
	Super::OnOutIdleState();
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetIdleState(false);
	}
	Get3PAnimInstance()->SetIdleState(false);
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
	bIsAiming ? CameraManager->OnAim(CurrentWeapon->GetWeaponAimTime()) : CameraManager->OnStopAim(CurrentWeapon->GetWeaponAimTime());
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetIsAiming(bIsAiming);
	}
	if (Get3PAnimInstance())
	{
		Get3PAnimInstance()->SetIsAiming(bIsAiming);
	}
}

void AINSPlayerCharacter::OnRep_IsCrouched()
{
	Super::OnRep_IsCrouched();
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetCurrentStance(ECharacterStance::CROUCH);
		Get1PAnimInstance()->SetIsCrouching(bIsCrouched);
	}
	Get3PAnimInstance()->SetCurrentStance(ECharacterStance::CROUCH);
	Get3PAnimInstance()->SetIsCrouching(bIsCrouched);
}

void AINSPlayerCharacter::OnRep_Sprint()
{
	Super::OnRep_Sprint();
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetSprintPressed(bIsSprint);
	}
	Get3PAnimInstance()->SetSprintPressed(bIsSprint);
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
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetAimHandIKXLocation(Value);
	}
}

bool AINSPlayerCharacter::CheckCharacterIsReady()
{
	if (Get1PAnimInstance() && Get1PAnimInstance()->GetIsAnimInitialized() && Get1PAnimInstance()->GetIsValidPlayAnim())
	{
		return true;
	}
	return false;
}

void AINSPlayerCharacter::CheckPendingEquipWeapon(float DeltaTimeSeconds)
{
	if (PendingWeaponEquipEvent.bIsEventActive)
	{
		PendingWeaponEquipEvent.PendingDuration = FMath::Clamp<float>(PendingWeaponEquipEvent.PendingDuration - DeltaTimeSeconds, 0.f, PendingWeaponEquipEvent.PendingDuration);
		if (PendingWeaponEquipEvent.PendingDuration == 0.f)
		{
			class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(PendingWeaponEquipEvent.WeaponClass, GetActorTransform(), GetOwner(), this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (NewWeapon)
			{
				NewWeapon->SetAutonomousProxy(true);
				NewWeapon->SetWeaponState(EWeaponState::NONE);
				NewWeapon->SetOwner(GetOwner());
				NewWeapon->SetOwnerCharacter(this);
				NewWeapon->SetInventorySlotIndex(PendingWeaponEquipEvent.WeaponSlotIndex);
				UGameplayStatics::FinishSpawningActor(NewWeapon, GetActorTransform());
			}
			SetCurrentWeapon(NewWeapon);
			PendingWeaponEquipEvent.bIsEventActive = false;
		}
	}
}

void AINSPlayerCharacter::HandleCrouchRequest()
{
	Super::HandleCrouchRequest();
}

void AINSPlayerCharacter::HandleItemEquipRequest(const uint8 SlotIndex)
{
	Super::HandleItemEquipRequest(SlotIndex);
	if(HasAuthority())
	{
		EquipItem(SlotIndex);
	}
	if(GetLocalRole()==ROLE_AutonomousProxy)
	{
		ServerEquipItem(SlotIndex);
	}
}

void AINSPlayerCharacter::HandleItemFinishUnEquipRequest()
{
	Super::HandleItemFinishUnEquipRequest();
}

void AINSPlayerCharacter::ServerEquipItem_Implementation(const uint8 SlotIndex)
{
	EquipItem(SlotIndex);
}

bool AINSPlayerCharacter::ServerEquipItem_Validate(const uint8 SlotIndex)
{
	return true;
}

void AINSPlayerCharacter::EquipItem(const uint8 SlotIndex)
{
	FInventorySlot* SelectedSlot = InventoryComp->GetItemSlot(SlotIndex);
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetInventorySlotIndex() != SlotIndex)
		{
			UnEquipItem();
			PendingWeaponEquipEvent.EventCreateTime = GetWorld()->GetTimeSeconds();
			PendingWeaponEquipEvent.PendingDuration = 1.f;
			PendingWeaponEquipEvent.WeaponClass = SelectedSlot->SlotWeaponClass;
			PendingWeaponEquipEvent.WeaponSlotIndex = SlotIndex;
			PendingWeaponEquipEvent.bIsEventActive = true;
		}
	}
	else
	{
		class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(SelectedSlot->SlotWeaponClass, GetActorTransform(), GetOwner(), this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (NewWeapon)
		{
			NewWeapon->SetAutonomousProxy(true);
			NewWeapon->SetWeaponState(EWeaponState::NONE);
			NewWeapon->SetOwner(GetOwner());
			NewWeapon->SetOwnerCharacter(this);
			NewWeapon->SetInventorySlotIndex(SlotIndex);
			UGameplayStatics::FinishSpawningActor(NewWeapon, GetActorTransform());
		}
		SetCurrentWeapon(NewWeapon);
		CurrentWeapon->CurrentClipAmmo = SelectedSlot->ClipAmmo;
		CurrentWeapon->AmmoLeft = SelectedSlot->AmmoLeft;
	}
}

void AINSPlayerCharacter::SetCurrentAnimData(UINSStaticAnimData* AnimData)
{
	Super::SetCurrentAnimData(AnimData);
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetCurrentWeaponAnimData(CurrentAnimPtr);
	}
	Get3PAnimInstance()->SetCurrentWeaponAnimData(CurrentAnimPtr);
}

void AINSPlayerCharacter::EquipGameModeDefaultWeapon()
{
	UClass* CurrentWeaponClass = GetINSPlayerController()->GetGameModeRandomWeapon();
	if (!CurrentWeaponClass)
	{
		return;
	}
	class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(CurrentWeaponClass, GetActorTransform(), GetOwner(), this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
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
	if(InventoryComp)
	{
		uint8 ItemSlotIdx = 0;
		UClass* BestWeaponClass = InventoryComp->GiveBestWeapon(ItemSlotIdx);
		if (!BestWeaponClass)
		{
			return;
		}
		class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(BestWeaponClass, GetActorTransform(), GetOwner(), this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (NewWeapon)
		{
			NewWeapon->SetAutonomousProxy(true);
			NewWeapon->SetWeaponState(EWeaponState::NONE);
			NewWeapon->SetOwner(GetOwner());
			NewWeapon->SetOwnerCharacter(this);
			NewWeapon->SetInventorySlotIndex(ItemSlotIdx);
			UGameplayStatics::FinishSpawningActor(NewWeapon, GetActorTransform());
		}
		SetCurrentWeapon(NewWeapon);
	}
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

void AINSPlayerCharacter::SetupMeshVisibility()
{
	CharacterMesh3P->SetOwnerNoSee(true);
	if (CharacterMesh1P)
	{
		CharacterMesh1P->SetOnlyOwnerSee(true);
	}
}

void AINSPlayerCharacter::UpdateCharacterMesh1P(float DeltaTime)
{
	if (GetLocalRole() >= ROLE_AutonomousProxy && !IsNetMode(NM_DedicatedServer))
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
		CapsuleTransFrom.SetLocation(CapsuleTransFrom.GetLocation() + FVector(-15.f * PitchOffSetMultiplier, 0.f, CurrentEyeHeight * PitchOffSetMultiplier * 0.35f));
		FTransform RelativeTransForm = UKismetMathLibrary::MakeRelativeTransform(Mesh1pPelvisTrans, CapsuleTransFrom);
		FVector RelLocation = RelativeTransForm.GetLocation();
		RelLocation.Y = 0.f;
		CharacterMesh1P->AddRelativeLocation(-RelLocation);
	}
}

void AINSPlayerCharacter::OnRep_Owner()
{
	Super::OnRep_Owner();
	INSPlayerController = Cast<AINSPlayerController>(GetOwner());
	if (!IsNetMode(NM_DedicatedServer))
	{
		bool bMyController = INSPlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (bMyController && CharacterAudioComp)
		{
			CharacterAudioComp->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_HeadSocket"));
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

FORCEINLINE UINSCharacterAimInstance* AINSPlayerCharacter::Get1PAnimInstance()
{
	if (CharacterMesh1P)
	{
		return Cast<UINSFPAnimInstance>(CharacterMesh1P->AnimScriptInstance);
	}
	return nullptr;
}

FORCEINLINE UINSCharacterAimInstance* AINSPlayerCharacter::Get3PAnimInstance()
{
	return Cast<UINSTPAnimInstance>(CharacterMesh3P->AnimScriptInstance);
}

void AINSPlayerCharacter::SetCurrentWeapon(class AINSWeaponBase* NewWeapon)
{
	Super::SetCurrentWeapon(NewWeapon);
}

void AINSPlayerCharacter::SetupWeaponAttachment()
{
	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		CurrentWeapon->GetWeaponMeshComp()->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	}
	else
	{
		CurrentWeapon->GetWeaponMeshComp()->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	}
}
