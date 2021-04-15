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
#include "Components./CapsuleComponent.h"
#include "Camera/CameraShake.h"
#include "INSCharacter/INSPlayerCameraManager.h"
#ifndef GEngine
#include "Engine/Engine.h"
#endif

AINSPlayerCharacter::AINSPlayerCharacter(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharSkeletalMeshComponent>(AINSPlayerCharacter::MeshComponentName))
{
	SetReplicates(true);
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
	//init animation view mode
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetViewMode(EViewMode::FPS);
	}
	if (Get3PAnimInstance())
	{
		Get3PAnimInstance()->SetViewMode(EViewMode::TPS);
	}
}

void AINSPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	FirstPersonCamera->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_CameraBoneSocket"));
	//sync 1p and 3p mesh first
}

void AINSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AINSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetOwner(Cast<AINSPlayerController>(GetOwner()));
	CurrentPlayerController = Cast<AINSPlayerController>(NewController);
	SetupMeshVisibility();
	if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
	{
		OnRep_TeamType();
	}
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		CharacterMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CharacterMesh1P->DestroyComponent(false);
	}
	UClass* CurrentWeaponClass = GetINSPlayerController()->GetGameModeRandomWeapon();
	if (!CurrentWeaponClass)
	{
		return;
	}
	class AINSWeaponBase* NewWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(CurrentWeaponClass
		, GetActorTransform()
		, GetINSPlayerController()
		, this
		, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (NewWeapon)
	{
		NewWeapon->SetAutonomousProxy(true);
		NewWeapon->SetWeaponState(EWeaponState::NONE);
		NewWeapon->SetOwner(NewController);
		NewWeapon->SetOwnerCharacter(this);
		UGameplayStatics::FinishSpawningActor(NewWeapon, GetActorTransform());

	}
	SetCurrentWeapon(NewWeapon);
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
		Get1PAnimInstance()->PlayWeaponStartEquipAnim();
		Get3PAnimInstance()->SetCurrentWeaponAndAnimationData(CurrentWeapon);
		Get3PAnimInstance()->PlayWeaponStartEquipAnim();
	}
	else
	{
		Get1PAnimInstance()->SetCurrentWeaponAndAnimationData(nullptr);
		Get3PAnimInstance()->SetCurrentWeaponAndAnimationData(nullptr);
	}
}

void AINSPlayerCharacter::CharacterEquipWeapon()
{
	if (CurrentWeapon)
	{

	}
}

void AINSPlayerCharacter::SetTeamType(const ETeamType NewTeamType)
{
	MyTeamType = NewTeamType;
}

void AINSPlayerCharacter::OnDeath()
{
	if (!GetIsCharacterDead())
	{
		SetLifeSpan(3.0f);
		bIsDead = true;
		if (GetLocalRole() == ROLE_Authority)
		{
			AINSPlayerStateBase* MyPlayerState = GetPlayerState<AINSPlayerStateBase>();
			if (MyPlayerState)
			{
				MyPlayerState->ReceivePlayerDeath(GetINSPlayerController());
			}
			SpawnWeaponPickup();
		}
		if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
		{
			OnRep_Dead();
		}
	}
}

void AINSPlayerCharacter::OnRep_Dead()
{
	Super::OnRep_Dead();
	CharacterMesh1P->SetHiddenInGame(true);
	CharacterMesh3P->SetHiddenInGame(false);
	CharacterMesh3P->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	CharacterMesh3P->AddImpulseAtLocation(LastHitInfo.Momentum * 0.4f, LastHitInfo.RelHitLocation);
	if (GetLocalRole() == ROLE_AutonomousProxy || GetLocalRole() == ROLE_Authority)
	{
		GetINSPlayerController()->OnCharacterDeath();
	}
}

void AINSPlayerCharacter::OnRep_Aim()
{
	Super::OnRep_Aim();
	if (!CurrentWeapon || GetNetMode() == ENetMode::NM_DedicatedServer
		|| !GetINSPlayerController())
	{
		return;
	}
	AINSPlayerCameraManager* CameraManager =
		Cast<AINSPlayerCameraManager>(GetINSPlayerController()->PlayerCameraManager);
	if (!CameraManager)
	{
		return;
	}
	bIsAiming ? CameraManager->OnAim(CurrentWeapon->GetWeaponAimTime())
		: CameraManager->OnStopAim(CurrentWeapon->GetWeaponAimTime());
	//update and perform aiming animation
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
	if (IsLocallyControlled() && LastHitInfo.Damage > 0)
	{
//		GetINSPlayerController()->ClientPlayCameraShake(TakeHitCameraShake);
	}
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
	/*if (GetLocalRole() == ROLE_Authority || GetLocalRole() == ROLE_AutonomousProxy)
	{
		CharacterMesh1P->SetHiddenInGame(false);
		CharacterMesh3P->SetHiddenInGame(true);
		CharacterMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		CharacterMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
		CharacterMesh1P->SetCastShadow(true);
		CharacterMesh1P->bCastDynamicShadow = true;
		CharacterMesh3P->SetCastShadow(true);
		CharacterMesh3P->bCastDynamicShadow = true;
	}
	else
	{
		CharacterMesh1P->SetHiddenInGame(true);
		CharacterMesh3P->SetHiddenInGame(false);
		CharacterMesh1P->SetComponentTickEnabled(false);
		CharacterMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		CharacterMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		CharacterMesh1P->SetCastShadow(false);
		CharacterMesh1P->bCastDynamicShadow = false;
		CharacterMesh3P->SetCastShadow(true);
		CharacterMesh3P->bCastDynamicShadow = true;
	}*/
	CharacterMesh3P->SetOwnerNoSee(true);
	CharacterMesh1P->SetOnlyOwnerSee(true);
}

void AINSPlayerCharacter::OnRep_Owner()
{
	Super::OnRep_Owner();
	CurrentPlayerController = Cast<AINSPlayerController>(GetOwner());
}

void AINSPlayerCharacter::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	OnRep_Owner();
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
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_CurrentWeapon();
	}
}

void AINSPlayerCharacter::SetupWeaponAttachment()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentWeapon->WeaponMesh1PComp->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
		CurrentWeapon->WeaponMesh3PComp->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	}
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		CurrentWeapon->WeaponMesh1PComp->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	}
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		CurrentWeapon->WeaponMesh3PComp->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	}
}

void AINSPlayerCharacter::SetINSPlayerController(class AINSPlayerController* NewPlayerController)
{
	this->CurrentPlayerController = NewPlayerController;
}
