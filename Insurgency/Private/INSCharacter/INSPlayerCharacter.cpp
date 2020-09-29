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
/*#include "Kismet/KismetMathLibrary.h"*/
#ifndef GEngine
#include "Engine/Engine.h"
#endif

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
	CameraArmComp->TargetArmLength = 0.f;
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
	if (CharacterTeam)
	{
		SetupPlayerMesh();
	}
	SetupCharacterRenderings();
	CharacterMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	CameraArmComp->AddRelativeLocation(FVector(20.f, 0.f, +CapsuleHalfHeight / 2.f + 20.f));
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
	DOREPLIFETIME(AINSPlayerCharacter, CharacterTeam);
}

void AINSPlayerCharacter::OnRep_CurrentWeapon()
{
	Super::OnRep_CurrentWeapon();
	if (CurrentWeapon)
	{
		if (GetINSPlayerController())
		{
			GetINSPlayerController()->SetCurrentWeapon(CurrentWeapon);
		}
		CurrentWeapon->SetupWeaponMeshRenderings();
		CharacterEquipWeapon();
	}
}

void AINSPlayerCharacter::CharacterEquipWeapon()
{
	if (CurrentWeapon)
	{
		Get1PAnimInstance()->SetCurrentWeaponRef(CurrentWeapon);
		Get3PAnimInstance()->SetCurrentWeaponRef(CurrentWeapon);
		CurrentWeapon->WeaponMesh1PComp->AttachToComponent(CharacterMesh1P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
		CurrentWeapon->WeaponMesh3PComp->AttachToComponent(CharacterMesh3P, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
	}
}

void AINSPlayerCharacter::UpdateADSHandsIkOffset()
{
	if (GetPlayerCameraComp() && CurrentWeapon)
	{
// 		const FTransform CameraPosition = GetPlayerCameraComp()->GetComponentTransform();
// 		FTransform WeaponSightTransform;
// 		CurrentWeapon->GetADSSightTransform(WeaponSightTransform);
// 		FTransform RelTrans = UKismetMathLibrary::MakeRelativeTransform(CameraPosition, WeaponSightTransform);
// 		CurrentWeapon->SetAdjustADSHandsIk(FVector(-10.f,-3.75f,RelTrans.GetLocation().Z));
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
	CharacterMesh3P->AddImpulseAtLocation(LastHitInfo.Momentum*0.4f, LastHitInfo.RelHitLocation);
	if (GetLocalRole() == ROLE_AutonomousProxy||GetLocalRole()==ROLE_Authority)
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
	Get1PAnimInstance()->SetSprintPressed(bIsSprint);
	Get3PAnimInstance()->SetSprintPressed(bIsSprint);
}

void AINSPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (GetLocalRole() == ROLE_AutonomousProxy)
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
		GetINSPlayerController()->ClientPlayCameraShake(TakeHitCameraShake);
	}
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

void AINSPlayerCharacter::OnRep_CharacterTeam()
{
	if (CharacterTeam)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
#if WITH_EDITOR&&!UE_BUILD_SHIPPING
			FString DebugMessage;
			DebugMessage.Append(TEXT("Charcter:"));
			DebugMessage.Append(GetName());
			DebugMessage.Append("'s Team info Replicated!");
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, DebugMessage);
#endif
		}
		SetupPlayerMesh();
		AINSPlayerController* PlayerController = Cast<AINSPlayerController>(GetController());
		if (PlayerController)
		{
// 			if (GetINSPlayerController()->GetLocalRole() == ROLE_AutonomousProxy)
// 			{
// 				GetINSPlayerController()->ServerGetGameModeRandomWeapon();
// 			}
			 if (GetINSPlayerController()->GetLocalRole() == ROLE_Authority)
			{
				GetINSPlayerController()->GetGameModeRandomWeapon();
			}
		}
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
	if (CharacterTeam&&CharacterTeam->GetTeamType() == ETeamType::ALLIE)
	{
		CharacterMesh1P->SetSkeletalMesh(CTDefaultMesh.Mesh1p);
		CharacterMesh3P->SetSkeletalMesh(CTDefaultMesh.Mesh3p);
	}
	if (CharacterTeam&&CharacterTeam->GetTeamType() == ETeamType::REBEL)
	{
		CharacterMesh1P->SetSkeletalMesh(TerroristDefaultMesh.Mesh1p);
		CharacterMesh3P->SetSkeletalMesh(TerroristDefaultMesh.Mesh3p);
	}
	SetupCharacterRenderings();
}

void AINSPlayerCharacter::SetupCharacterRenderings()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetNetMode() == ENetMode::NM_DedicatedServer)
		{
			CharacterMesh1P->SetHiddenInGame(true);
			CharacterMesh3P->SetHiddenInGame(true);
			CharacterMesh1P->SetComponentTickEnabled(false);
			CharacterMesh3P->SetComponentTickEnabled(false);
			CharacterMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			CharacterMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			CharacterMesh1P->SetCastShadow(false);
			CharacterMesh1P->bCastDynamicShadow = false;
			CharacterMesh3P->SetCastShadow(false);
			CharacterMesh3P->bCastDynamicShadow = false;
		}
		else if (GetNetMode() == ENetMode::NM_ListenServer || GetNetMode() == ENetMode::NM_Standalone)
		{
			if (this == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
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
				CharacterMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
				CharacterMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
				CharacterMesh1P->SetCastShadow(false);
				CharacterMesh1P->bCastDynamicShadow = false;
				CharacterMesh3P->SetCastShadow(true );
				CharacterMesh3P->bCastDynamicShadow = true;
			}
		}
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
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
	else if (GetLocalRole() == ROLE_SimulatedProxy)
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
	}
}

void AINSPlayerCharacter::SetCharacterTeam(class AINSTeamInfo* NewTeam)
{
	CharacterTeam = NewTeam;
	if (ROLE_Authority)
	{
		OnRep_CharacterTeam();
	}
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
	return PlayerCameraComp->GetComponentTransform();
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

void AINSPlayerCharacter::SetINSPlayerController(class AINSPlayerController* NewPlayerController)
{
	this->CurrentPlayerController = NewPlayerController;
}
