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
#ifndef GEngine
#include "Engine/Engine.h"
#endif

AINSPlayerCharacter::AINSPlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharSkeletalMeshComponent>(MeshComponentName))
{
	bReplicates = true;
	SetReplicatingMovement(true);
	GetCapsuleComponent()->SetCapsuleHalfHeight(86.f);
	GetCapsuleComponent()->SetCapsuleRadius(36.f);
	BaseEyeHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	CrouchedEyeHeight = BaseEyeHeight - BaseEyeHeight * 0.4f;
	CurrentEyeHeight = BaseEyeHeight;
	RootComponent = GetCapsuleComponent();
	FirstPersonCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	SpringArm = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("SpringArmComp"));
	SpringArmAligner = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SpringArmDummyAligner"));
	SpringArmAligner->SetupAttachment(RootComponent);
	SpringArm->SetupAttachment(SpringArmAligner);
	SpringArm->TargetArmLength = 0.1f;
	SpringArm->bUsePawnControlRotation = true;
	CharacterMesh3P = Cast<UINSCharSkeletalMeshComponent>(GetMesh());
	CharacterMesh3P->AddRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
	CharacterMesh1P = ObjectInitializer.CreateDefaultSubobject<UINSCharSkeletalMeshComponent>(this, TEXT("Character1pMeshComp"));
	InventoryComp = ObjectInitializer.CreateDefaultSubobject<UINSInventoryComponent>(this, TEXT("InventoryComp"));
	CharacterMesh1P->SetupAttachment(SpringArm);
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
	SpringArmAligner->AddRelativeLocation(FVector(0.f, 0.f, CurrentEyeHeight));
	CharacterMesh1P->AddRelativeLocation(FVector(0.f, 0.f, -174.f));
	bIsDead = false;
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	bShowDebugTrace = false;
#endif
}

void AINSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetupMeshVisibility();
	if (GetLocalRole() == ROLE_Authority || GetLocalRole() == ROLE_AutonomousProxy && !IsNetMode(NM_DedicatedServer))
	{
		CharacterReadyTick.bCanEverTick = true;
		CharacterReadyTick.TickInterval = 0.3f;
		CharacterReadyTick.SetTickFunctionEnable(true);
		CharacterReadyTick.Target = this;
		CharacterReadyTick.RegisterTickFunction(GetLevel());
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
}

void AINSPlayerCharacter::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if(&ThisTickFunction==&CharacterReadyTick)
	{
		if(CheckCharacterIsReady())
		{
			if(CurrentWeapon)
			{
				if(HasAuthority())
				{
					CurrentWeapon->SetWeaponState(EWeaponState::EQUIPPING);
				}else
				{
					CurrentWeapon->ServerSetWeaponState(EWeaponState::EQUIPPING);
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
	if(IsNetMode(NM_DedicatedServer))
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
			AINSPlayerController* const PC = PlayerCharacter->GetINSPlayerController();
			if (PC)
			{
				PC->PlayerCauseDamage(LastHitInfo);
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
	if(HasAuthority())
	{
		EquipGameModeDefaultWeapon();
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
		Get1PAnimInstance()->SetCurrentWeapon(CurrentWeapon);
		Get3PAnimInstance()->SetCurrentWeapon(CurrentWeapon);
		SetCurrentAnimData(CurrentWeapon->GetWeaponAnimDataPtr());
		Get1PAnimInstance()->SetBaseHandsIkLocation(CurrentWeapon->GetWeaponBaseIKLocation());
		Get3PAnimInstance()->SetBaseHandsIkLocation(CurrentWeapon->GetWeaponBaseIKLocation());
		Get1PAnimInstance()->SetAimHandIKXLocation(CurrentWeapon->GetWeaponAimHandIKXLocation());
	}
	else
	{
		Get1PAnimInstance()->SetCurrentWeapon(nullptr);
		Get3PAnimInstance()->SetCurrentWeapon(nullptr);
		SetCurrentAnimData(nullptr);
		Get1PAnimInstance()->SetBaseHandsIkLocation(FVector::ZeroVector);
		Get3PAnimInstance()->SetBaseHandsIkLocation(FVector::ZeroVector);
		Get1PAnimInstance()->SetAimHandIKXLocation(0.f);
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
		Get1PAnimInstance()->SetIdleState(true);
		Get3PAnimInstance()->SetIdleState(true);
	}
	if (GetCurrentWeapon() && GetCurrentWeapon()->GetCurrentWeaponState() == EWeaponState::IDLE)
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character:%s with weapon %s is not moving and not using weapon for some time ,enter idle state"), *GetName(), *(GetCurrentWeapon()->GetName()));
		Get1PAnimInstance()->SetIdleState(true);
		Get3PAnimInstance()->SetIdleState(true);
	}
}

void AINSPlayerCharacter::OnEnterBoredState()
{
	Super::OnEnterBoredState();
	if (!GetCurrentWeapon())
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character:%s with no weapon is not moving for too much time ,enter bored state"), *GetName());
		Get1PAnimInstance()->SetBoredState(true);
		Get3PAnimInstance()->SetBoredState(true);
	}
	if (GetCurrentWeapon() && GetCurrentWeapon()->GetCurrentWeaponState() == EWeaponState::IDLE)
	{
		UE_LOG(LogINSCharacter, Log, TEXT("Character:%s with weapon %s is not moving and not using weapon for too much time ,enter bored state"), *GetName(),*(GetCurrentWeapon()->GetName()));
		Get1PAnimInstance()->SetBoredState(true);
		Get3PAnimInstance()->SetBoredState(true);
	}
}


void AINSPlayerCharacter::OnOutBoredState()
{
	Get1PAnimInstance()->SetBoredState(false);
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
			CharacterMesh1P->SetHiddenInGame(true);
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
	Get1PAnimInstance()->SetWeaponBasePoseType(NewType);
	Get3PAnimInstance()->SetWeaponBasePoseType(NewType);
}

void AINSPlayerCharacter::OnOutIdleState()
{
	Super::OnOutIdleState();
	Get1PAnimInstance()->SetIdleState(false);
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
	Get1PAnimInstance()->SetCurrentStance(ECharacterStance::CROUCH);
	Get3PAnimInstance()->SetCurrentStance(ECharacterStance::CROUCH);
	Get1PAnimInstance()->SetIsCrouching(bIsCrouched);
	Get3PAnimInstance()->SetIsCrouching(bIsCrouched);
}

void AINSPlayerCharacter::OnRep_Sprint()
{
	Super::OnRep_Sprint();
	Get1PAnimInstance()->SetSprintPressed(bIsSprint);
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

void AINSPlayerCharacter::SetAimHandsXLocation(const float Value)
{
	if(Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetAimHandIKXLocation(Value);
	}
}

bool AINSPlayerCharacter::CheckCharacterIsReady()
{
	if(Get1PAnimInstance()->GetIsAnimInitialized()&&Get1PAnimInstance()->GetIsValidPlayAnim())
	{
		return true;
	}
	return false;
}

void AINSPlayerCharacter::HandleCrouchRequest()
{
	Super::HandleCrouchRequest();
}

void AINSPlayerCharacter::HandleItemEquipRequest(const uint8 SlotIndex)
{
	Super::HandleItemEquipRequest(SlotIndex);
	if (HasAuthority() && InventoryComp)
	{
		if (CurrentWeapon && CurrentWeapon->GetInventorySlotIndex() != SlotIndex)
		{
			const bool PutSuccess = InventoryComp->PutItemInSlot(CurrentWeapon);
			if (PutSuccess)
			{
				CurrentWeapon->Destroy();
				CurrentWeapon = nullptr;
			}
		}
		FInvetorySlot* SelectedSlot = InventoryComp->GetItemSlot(SlotIndex);
		if (SelectedSlot)
		{
			UClass* ItemClass = SelectedSlot->SlotWeaponClass;
			if (ItemClass)
			{
				class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(ItemClass, GetActorTransform(), GetOwner(), this , ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
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
			}
		}
	}
}

void AINSPlayerCharacter::PutCurrentWeaponBackToSlot()
{
	if (CurrentWeapon)
	{
		//FInvetorySlot* TargetSlot = InventoryComp->
	}
}

void AINSPlayerCharacter::SetCurrentAnimData(UINSStaticAnimData* AnimData)
{
	Super::SetCurrentAnimData(AnimData);
	Get1PAnimInstance()->SetCurrentWeaponAnimData(CurrentAnimPtr);
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
	CharacterMesh1P->SetOnlyOwnerSee(true);
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


bool AINSPlayerCharacter::GetIsMesh1pHidden() const
{
	return CharacterMesh1P->bHiddenInGame;
}

bool AINSPlayerCharacter::GetIsMesh3pHidden() const
{
	return CharacterMesh3P->bHiddenInGame;
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
		//OutCameraSocketTransform = CharacterMesh1P->GetSocketTransform("Bip01_CameraBoneSocket", RTS_Component);
		OutCameraSocketTransform = FirstPersonCamera->GetComponentTransform();
	}
	else
	{
		OutCameraSocketTransform = FTransform::Identity;
	}
}

FORCEINLINE UINSCharacterAimInstance* AINSPlayerCharacter::Get1PAnimInstance()
{
	return Cast<UINSFPAnimInstance>(CharacterMesh1P->AnimScriptInstance);
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
	CurrentWeapon->WeaponMesh1PComp->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	CurrentWeapon->WeaponMesh3PComp->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
}
