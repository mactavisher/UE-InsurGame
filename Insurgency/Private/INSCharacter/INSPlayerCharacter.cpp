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

AINSPlayerCharacter::AINSPlayerCharacter(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer.SetDefaultSubobjectClass<UINSCharSkeletalMeshComponent>(AINSPlayerCharacter::MeshComponentName))
{
	PlayerCameraComp = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("PlayerCameraComp"));
	CameraArmComp = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("PlayerArmComp"));
	CharacterMesh3P = Cast<UINSCharSkeletalMeshComponent>(GetMesh());
	CharacterMesh1P = ObjectInitializer.CreateDefaultSubobject<UINSCharSkeletalMeshComponent>(this, TEXT("Character1pMeshComp"));
	CameraArmComp->SetupAttachment(RootComponent);
	PlayerCameraComp->SetupAttachment(CameraArmComp);
	CharacterMesh1P->SetupAttachment(CameraArmComp);
	CharacterMesh1P->bCastDynamicShadow = false;
	CharacterMesh1P->CastShadow = false;
	CharacterMesh1P->bReceivesDecals = false;
	CharacterMesh1P->LightingChannels.bChannel1 = true;
	CameraArmComp->TargetArmLength = 0.1f;
	CameraArmComp->bUsePawnControlRotation = true;
	CharacterMesh3P->SetupAttachment(RootComponent);
	CharacterMesh3P->AlwaysLoadOnClient = true;
	CharacterMesh3P->AlwaysLoadOnServer = true;
	CharacterMesh1P->AlwaysLoadOnClient = true;
	CharacterMesh1P->AlwaysLoadOnServer = true;
	CharacterMesh3P->SetUsingAbsoluteRotation(false);
	CharacterMesh3P->bReceivesDecals = false;
	CharacterMesh3P->bLightAttachmentsAsGroup = true;
	CharacterMesh3P->LightingChannels.bChannel1 = true;
	CharacterMesh3P->bCastCapsuleIndirectShadow = true;
	SetReplicates(true);
	SetReplicateMovement(true);
	BaseEyeHeight = DefaultBaseEyeHeight;
	CurrentEyeHeight = BaseEyeHeight;
	//sync 1p and 3p mesh first
	CharacterMesh3P->AddRelativeLocation(FVector(0.f, 0.f, -BaseEyeHeight));
	//location off set with camera arm and 1p mesh comp
	CrouchedEyeHeight = BaseEyeHeight * 0.6f;
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	bShowDebugTrace = false;
#endif
}

void AINSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetupPlayerMesh();
	CharacterMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterMesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	CharacterMesh3P->SetHiddenInGame(true);
	CharacterMesh1P->SetHiddenInGame(true);
	if (IsLocallyControlled())
	{
		CharacterMesh1P->SetHiddenInGame(false);
	}
	else
	{
		CharacterMesh3P->SetHiddenInGame(false);
	}
	CharacterMesh3P->CastShadow = true;
	CharacterMesh1P->CastShadow = false;
	CharacterMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	CharacterMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
}

void AINSPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//init animation view mode
	if (Get1PAnimInstance())
	{
		Get1PAnimInstance()->SetViewMode(EViewMode::FPS);
	}
	if (Get3PAnimInstance())
	{
		Get3PAnimInstance()->SetViewMode(EViewMode::TPS);
	}
	//CharacterMesh3P->SetWorldLocationAndRotation(GetActorLocation(), GetActorRotation());
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	CameraArmComp->AddRelativeLocation(FVector(20.f, 0.f, +CapsuleHalfHeight / 2.f+20.f));
	CharacterMesh1P->AddRelativeLocation(FVector(-5.f, -10.2f, -(BaseEyeHeight + 58.f)));
}

void AINSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsLocallyControlled())
	{
		SimulateViewTrace();
	}
}

void AINSPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetOwner(Cast<AINSPlayerController>(GetOwner()));
}

void AINSPlayerCharacter::OnThreatenSpoted(AActor* ThreatenActor, AController* ThreatenInstigator)
{
	if (ThreatenInstigator&&ThreatenInstigator->GetClass()->IsChildOf(AINSPlayerController::StaticClass()))
	{
		//TODO, team role check
		const AINSPlayerCharacter* const PlayerCharacter = Cast<AINSPlayerCharacter>(ThreatenActor);
		if (PlayerCharacter && !PlayerCharacter->GetIsCharacterDead())
		{
			Cast<AINSPlayerController>(GetController())->ClientShowThreaten();
		}
	}
}

void AINSPlayerCharacter::SimulateViewTrace()
{
	const float ViewTraceRange = 10000.f;
	FCollisionQueryParams ViewTraceQueryParams;
	ViewTraceQueryParams.AddIgnoredActor(this);
	FHitResult ViewTraceHit(ForceInit);
	const AINSPlayerController* PlayerController = CastChecked<AINSPlayerController>(GetOwner());
	FVector ViewLoc;
	FRotator ViewRot;
	const FVector TraceEnd = ViewLoc + PlayerController->GetControlRotation().Vector() * ViewTraceRange;
	PlayerController->GetPlayerViewPoint(ViewLoc, ViewRot);
	GetWorld()->LineTraceSingleByChannel(ViewTraceHit, ViewLoc, TraceEnd, ECollisionChannel::ECC_Camera, ViewTraceQueryParams);
	if (ViewTraceHit.bBlockingHit && ViewTraceHit.GetActor())
	{
		if (ViewTraceHit.GetActor()->GetClass()->IsChildOf(AINSPlayerCharacter::StaticClass()))
		{
			//UE_LOG(LogINSCharacter, Warning, TEXT("Threaten player:%s spotted"), *ViewTraceHit.GetActor()->GetName());
			OnThreatenSpoted(ViewTraceHit.GetActor(), CastChecked<AINSPlayerCharacter>(ViewTraceHit.GetActor())->GetController());
		}
	}
#if WITH_EDITORONLY_DATA&&!UE_BUILD_SHIPPING
	if (bShowDebugTrace)
	{
		DrawDebugLine(GetWorld(), ViewLoc, TraceEnd, FColor::Red, false, 1.f);
	}
#endif
}

void AINSPlayerCharacter::UpdateCrouchedEyeHeight(float DeltaTimeSeconds)
{
	if (bIsCrouched)
	{
		BaseEyeHeight -= 12.f*DeltaTimeSeconds;
		if (BaseEyeHeight <= CrouchedEyeHeight)
		{
			BaseEyeHeight = CrouchedEyeHeight;
		}
	}
}

AINSPlayerController* AINSPlayerCharacter::GetINSPlayerController()
{
	return CastChecked<AINSPlayerController>(GetController());
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
}

void AINSPlayerCharacter::OnRep_CurrentWeapon()
{
	Super::OnRep_CurrentWeapon();
	Get1PAnimInstance()->SetCurrentWeaponRef(CurrentWeapon);
	Get3PAnimInstance()->SetCurrentWeaponRef(CurrentWeapon);
	CurrentWeapon->WeaponMesh1PComp->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	CurrentWeapon->WeaponMesh3PComp->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentWeapon->WeaponMesh1PComp->SetHiddenInGame(true);
		CurrentWeapon->WeaponMesh3PComp->SetHiddenInGame(true);
		if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
		{
			CurrentWeapon->WeaponMesh1PComp->SetHiddenInGame(false);
			CurrentWeapon->WeaponMesh3PComp->SetHiddenInGame(true);
		}
	}
	if (IsLocallyControlled())
	{
		CurrentWeapon->WeaponMesh1PComp->SetHiddenInGame(false);
		CurrentWeapon->WeaponMesh3PComp->SetHiddenInGame(true);
	}
	else if (!IsLocallyControlled())
	{
		CurrentWeapon->WeaponMesh1PComp->SetHiddenInGame(true);
		CurrentWeapon->WeaponMesh3PComp->SetHiddenInGame(false);
	}
	
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
	CharacterAudioComp->SetVoiceType(EVoiceType::DIE);
	CharacterAudioComp->PlayVoice();
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		GetINSPlayerController()->OnCharacterDeath();
	}
}

void AINSPlayerCharacter::OnRep_IsCrouched()
{
	Super::OnRep_IsCrouched();
	Get1PAnimInstance()->SetCurrentStance(ECharacterStance::CROUCH);
	Get3PAnimInstance()->SetCurrentStance(ECharacterStance::CROUCH);
	
}

void AINSPlayerCharacter::OnRep_Sprint()
{
	Super::OnRep_Sprint();
}

void AINSPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	SetupPlayerMesh();
}

void AINSPlayerCharacter::OnRep_LastHitInfo()
{
	Super::OnRep_LastHitInfo();
	if (IsLocallyControlled()&&LastHitInfo.Damage>0)
	{
		GetINSPlayerController()->ClientPlayCameraShake(TakeHitCameraShake);
	}
}

void AINSPlayerCharacter::UpdateCrouchEyeHeightSmoothly()
{
	const float UpdateDeltaValue = 1.f;
	if (bIsCrouched)
	{
		CurrentEyeHeight -= UpdateDeltaValue;
		if (CurrentEyeHeight <= CrouchedEyeHeight)
		{
			CrouchedEyeHeight = CrouchedEyeHeight;
		}
	}
	else
	{
		CurrentEyeHeight += 1.f;
	}
	
}

void AINSPlayerCharacter::SetupPlayerMesh()
{
	const AINSPlayerStateBase* const INSPlayerState = GetPlayerState<AINSPlayerStateBase>();
	if (INSPlayerState)
	{
		const AINSTeamInfo* PlayerTeam = INSPlayerState->GetPlayerTeam();
		if (PlayerTeam&&PlayerTeam->GetTeamType() == ETeamType::CT)
		{
			CharacterMesh1P->SetSkeletalMesh(CTDefaultMesh.Mesh1p);
			CharacterMesh3P->SetSkeletalMesh(CTDefaultMesh.Mesh3p);
		}
		if (PlayerTeam&&PlayerTeam->GetTeamType() == ETeamType::T)
		{
			CharacterMesh1P->SetSkeletalMesh(TerroristDefaultMesh.Mesh1p);
			CharacterMesh3P->SetSkeletalMesh(TerroristDefaultMesh.Mesh3p);
		}
	}
	CharacterSetupFinished.Broadcast();
}

void AINSPlayerCharacter::ReceiveFriendlyFire(class AINSPlayerController* InstigatorPlayer, float DamageTaken)
{

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
		CurrentWeapon->StartEquipWeapon();
	}
}

void AINSPlayerCharacter::SetINSPlayerController(class AINSPlayerController* NewPlayerController)
{
	this->CurrentPlayerController = NewPlayerController;
}
