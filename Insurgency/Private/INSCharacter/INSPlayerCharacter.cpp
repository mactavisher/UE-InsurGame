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
#include "INSCharacter/INSPlayerCameraManager.h"
#include "INSComponents/INSInventoryComponent.h"
#ifndef GEngine
#include "Engine/Engine.h"
#endif

AINSPlayerCharacter::AINSPlayerCharacter(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharSkeletalMeshComponent>(AINSPlayerCharacter::MeshComponentName))
{
	bReplicates = true;
	SetReplicateMovement(true);
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
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	bShowDebugTrace = false;
#endif
}

void AINSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetupMeshVisibility();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(EquipDefaultWeaponHandle, this, &AINSPlayerCharacter::EquipGameModeDefaultWeapon, 1.f, false, 0.1f);
	}
}

void AINSPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	FirstPersonCamera->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_CameraBoneSocket"));
}

void AINSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AINSPlayerCharacter::OnCauseDamage(const FTakeHitInfo& HitInfo)
{
	Super::OnCauseDamage(HitInfo);
	if (!IsNetMode(NM_DedicatedServer))
	{
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
}

void AINSPlayerCharacter::PossessedBy(AController* NewController)
{
	CurrentPlayerController = Cast<AINSPlayerController>(NewController);
	Super::PossessedBy(NewController);
	SetupMeshVisibility();
	if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
	{
		OnRep_TeamType();
	}
}

AINSPlayerController* AINSPlayerCharacter::GetINSPlayerController()
{
	return CurrentPlayerController;
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
		Get1PAnimInstance()->SetCurrentWeaponAndAnimationData(CurrentWeapon);
		Get3PAnimInstance()->SetCurrentWeaponAndAnimationData(CurrentWeapon);
		Get1PAnimInstance()->PlayWeaponStartEquipAnim();
		Get3PAnimInstance()->PlayWeaponStartEquipAnim();
	}
	else
	{
		Get1PAnimInstance()->SetCurrentWeaponAndAnimationData(nullptr);
		Get3PAnimInstance()->SetCurrentWeaponAndAnimationData(nullptr);
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
		UE_LOG(LogINSCharacter
			, Log
			, TEXT("Character:%s with no weapon is not moving for some time ,enter idle state")
			, *GetName());
		Get1PAnimInstance()->SetIdleState(true);
		Get3PAnimInstance()->SetIdleState(true);
	}
	if (GetCurrentWeapon() && GetCurrentWeapon()->GetCurrentWeaponState() == EWeaponState::IDLE)
	{
		UE_LOG(LogINSCharacter
			, Log, TEXT("Character:%s with weapon %s is not moving and not using weapon for some time ,enter idle state")
			, *GetName()
			, *(GetCurrentWeapon()->GetName()));
		Get1PAnimInstance()->SetIdleState(true);
		Get3PAnimInstance()->SetIdleState(true);
	}
}

void AINSPlayerCharacter::OnEnterBoredState()
{
	Super::OnEnterBoredState();
	if (!GetCurrentWeapon())
	{
		UE_LOG(LogINSCharacter
			, Log
			, TEXT("Character:%s with no weapon is not moving for too much time ,enter bored state")
			, *GetName());
		Get1PAnimInstance()->SetBoredState(true);
		Get3PAnimInstance()->SetBoredState(true);
	}
	if (GetCurrentWeapon() && GetCurrentWeapon()->GetCurrentWeaponState() == EWeaponState::IDLE)
	{
		UE_LOG(LogINSCharacter
			, Log
			, TEXT("Character:%s with weapon %s is not moving and not using weapon for too much time ,enter bored state")
			, *GetName(),
			*(GetCurrentWeapon()->GetName()));
		Get1PAnimInstance()->SetBoredState(true);
		Get3PAnimInstance()->SetBoredState(true);
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

void AINSPlayerCharacter::SetWeaponBasePoseType(const EWeaponBasePoseType NewType)
{
	Get1PAnimInstance()->SetWeaponBasePoseType(NewType);
	Get3PAnimInstance()->SetWeaponBasePoseType(NewType);
}

void AINSPlayerCharacter::OnOutIdleState()
{
	Super::OnOutIdleState();
}

void AINSPlayerCharacter::OnDeath()
{
	if (HasAuthority() && !GetIsDead())
	{
		bIsDead = true;
		SetLifeSpan(10.f);
		AINSPlayerStateBase* MyPlayerState = GetPlayerState<AINSPlayerStateBase>();
		if (MyPlayerState)
		{
			MyPlayerState->ReceivePlayerDeath(GetINSPlayerController());
		}
		OnRep_Dead();
		SpawnWeaponPickup();
	}
}

void AINSPlayerCharacter::OnRep_Dead()
{
	Super::OnRep_Dead();
	if (!IsNetMode(NM_DedicatedServer))
	{
		CharacterMesh1P->SetHiddenInGame(true);
		CharacterMesh3P->SetHiddenInGame(false);
		CharacterMesh3P->SetSimulatePhysics(true);
	}
	else
	{
		CharacterMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CharacterMesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	CharacterMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterMesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AINSPlayerCharacter::OnRep_Aim()
{
	Super::OnRep_Aim();
	if (!CurrentWeapon || GetNetMode() == ENetMode::NM_DedicatedServer || !GetINSPlayerController())
	{
		return;
	}
	AINSPlayerCameraManager* CameraManager = Cast<AINSPlayerCameraManager>(GetINSPlayerController()->PlayerCameraManager);
	if (!CameraManager)
	{
		return;
	}
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
	if (CharacterMesh3P && GetIsDead())
	{
		CharacterMesh3P->SetSimulatePhysics(true);
	}
	else
	{
		OnCauseDamage(LastHitInfo);
	}
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
	if (LastHitInfo.Victim == this) {
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
				class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(ItemClass
					, GetActorTransform()
					, GetOwner()
					, this
					, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
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

void AINSPlayerCharacter::EquipGameModeDefaultWeapon()
{
	UClass* CurrentWeaponClass = GetINSPlayerController()->GetGameModeRandomWeapon();
	if (!CurrentWeaponClass)
	{
		return;
	}
	class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(CurrentWeaponClass
		, GetActorTransform()
		, GetOwner()
		, this
		, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
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
	//update autonomous client
	CurrentPlayerController = Cast<AINSPlayerController>(GetOwner());
	if (!IsNetMode(NM_DedicatedServer)) {
		bool bMyController = CurrentPlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0);
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
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_Owner();
	}
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
		OutCameraSocketTransform = CharacterMesh1P->GetSocketTransform("Bip01_CameraBoneSocket", RTS_World);
	}
	else
	{
		OutCameraSocketTransform = FTransform::Identity;
	}
}

FORCEINLINE UINSCharacterAimInstance* AINSPlayerCharacter::Get1PAnimInstance()
{
	return Cast<UINSCharacterAimInstance>(CharacterMesh1P->AnimScriptInstance);
}

FORCEINLINE UINSCharacterAimInstance* AINSPlayerCharacter::Get3PAnimInstance()
{
	return Cast<UINSCharacterAimInstance>(CharacterMesh3P->AnimScriptInstance);
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

void AINSPlayerCharacter::SetINSPlayerController(class AINSPlayerController* NewPlayerController)
{
	this->CurrentPlayerController = NewPlayerController;
}
