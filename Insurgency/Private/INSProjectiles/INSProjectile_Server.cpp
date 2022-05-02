// Fill out your copyright notice in the Description page of Project Settings.


#include "INSProjectiles/INSProjectile_Server.h"
#include "Components/SphereComponent.h"
#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "INSCharacter/INSPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSProjectiles/INSProjectile_Visual.h"
#include "INSComponents/INSProjectileMovementComponent.h"
#include "INSDamageTypes/INSDamageType_Projectile.h"
#include "Kismet/GameplayStatics.h"
DEFINE_LOG_CATEGORY(LogServerProjectile);
// Sets default values
AINSProjectile_Server::AINSProjectile_Server(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MovementQuantizeLevel = EVectorQuantization::RoundWholeNumber;
	VisualProjectile = nullptr;
	DamageFalloffCurve = nullptr;
	HitCounter = static_cast<uint8>(0);
}

// Called when the game starts or when spawned
void AINSProjectile_Server::BeginPlay()
{
	RegisterInitialTick(true);
	Super::BeginPlay();
	RegisterClientVisualProjectileTick(true);
}

void AINSProjectile_Server::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AINSProjectile_Server, HitCounter);
	DOREPLIFETIME(AINSProjectile_Server, OwnerWeapon);
	DOREPLIFETIME(AINSProjectile_Server, CurrentPenetrateCount);
	DOREPLIFETIME(AINSProjectile_Server, bScanTraceProjectile);
	DOREPLIFETIME(AINSProjectile_Server, ScanTraceHitLoc);
	DOREPLIFETIME_CONDITION(AINSProjectile_Server, MovementQuantizeLevel, COND_InitialOnly);
}

void AINSProjectile_Server::UpdateCollisionSettings()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}

void AINSProjectile_Server::PostNetReceiveLocationAndRotation()
{
	Super::PostNetReceiveLocationAndRotation();
	SetActorLocationAndRotation(GetReplicatedMovement().Location, GetReplicatedMovement().Rotation);
}

void AINSProjectile_Server::GatherCurrentMovement()
{
	if (HasAuthority() && !bScanTraceProjectile && RootComponent != nullptr && !GetValid(this) && !
		IsPendingKillPending())
	{
		// If we are attached, don't replicate absolute position
		if (RootComponent->GetAttachParent() != nullptr)
		{
			Super::GatherCurrentMovement();
		}
		else
		{
			FRepMovement OptRepMovement;
			OptRepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(RootComponent->GetComponentLocation(), this);
			OptRepMovement.Rotation = RootComponent->GetComponentRotation();
			OptRepMovement.LinearVelocity = GetVelocity();
			OptRepMovement.AngularVelocity = FVector::ZeroVector;
			OptRepMovement.bRepPhysics = true;
			OptRepMovement.LocationQuantizationLevel = MovementQuantizeLevel;
			//OptRepMovement.RotationQuantizationLevel = ERotatorQuantization::ShortComponents;
			OptRepMovement.VelocityQuantizationLevel = MovementQuantizeLevel;
			SetReplicatedMovement(OptRepMovement);
			LastMovementRepTime = GetWorld()->GetRealTimeSeconds();
		}
	}
}

void AINSProjectile_Server::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	DOREPLIFETIME_ACTIVE_OVERRIDE(AINSProjectile_Server, RepHitInfo, !bScanTraceProjectile);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void AINSProjectile_Server::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
}

void AINSProjectile_Server::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UpdateCollisionSettings();
}

void AINSProjectile_Server::RegisterClientVisualProjectileTick(bool bDoRegister)
{
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	CreateClientVisualProjectileTick.bCanEverTick = bDoRegister;
	CreateClientVisualProjectileTick.SetTickFunctionEnable(bDoRegister);
	if (bDoRegister)
	{
		CreateClientVisualProjectileTick.Target = this;
		CreateClientVisualProjectileTick.RegisterTickFunction(GetLevel());
	}
	else
	{
		CreateClientVisualProjectileTick.Target = nullptr;
		CreateClientVisualProjectileTick.UnRegisterTickFunction();
	}
}

void AINSProjectile_Server::RegisterInitialTick(bool bDoRegister)
{
	if (HasAuthority())
	{
		InitRepTickFunc.bCanEverTick = bDoRegister;
		InitRepTickFunc.SetTickFunctionEnable(bDoRegister);
		if (bDoRegister)
		{
			InitRepTickFunc.RegisterTickFunction(GetLevel());
			InitRepTickFunc.Target = this;
			PrimaryActorTick.AddPrerequisite(this, InitRepTickFunc);
		}
		else
		{
			InitRepTickFunc.UnRegisterTickFunction();
			InitRepTickFunc.Target = nullptr;
			PrimaryActorTick.RemovePrerequisite(this, InitRepTickFunc);
		}
	}
}

void AINSProjectile_Server::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (&ThisTickFunction == &InitRepTickFunc)
	{
		SendInitialReplication();
		RegisterInitialTick(false);
	}
	if (&ThisTickFunction == &CreateClientVisualProjectileTick)
	{
		UE_LOG(LogServerProjectile, Log, TEXT("Projectile creating function visual ticking"));
		if (bScanTraceConditionSet && OwnerWeapon && ScanTraceMoveTime > 0.f && !ScanTraceHitLoc.IsZero())
		{
			CreateVisualProjectile();
			RegisterClientVisualProjectileTick(false);
		}
	}
}

void AINSProjectile_Server::OnRep_HitCounter()
{
}

void AINSProjectile_Server::OnRep_ScanTraceCondition()
{
	bScanTraceConditionSet = true;
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->SetScanTraceProjectile(bScanTraceProjectile);
	}
}

void AINSProjectile_Server::OnRep_OwnerWeapon()
{
	if (VisualProjectile)
	{
		VisualProjectile->SetOwnerWeapon(OwnerWeapon);
	}
}

void AINSProjectile_Server::OnRep_ScanTraceTime()
{
	Super::OnRep_ScanTraceTime();
	ProjectileMovementComponent->SetScanTraceMoveTime(ScanTraceMoveTime);
}

void AINSProjectile_Server::OnRep_Hit()
{
}

void AINSProjectile_Server::OnRep_ScanTraceHitLoc()
{
}

void AINSProjectile_Server::OnRep_RepHitInfo()
{
	NotifyVisualProjectileHit();
}

void AINSProjectile_Server::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                            const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority && !bIsProcessingHit)
	{
		TGuardValue<bool> HitGuard(bIsProcessingHit, true);
		ProjectileMovementComponent->EnableTick(false);
		ProjectileMovementComponent->StopMovementImmediately();
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		HitCounter++;
		GatherCurrentMovement();
		UE_LOG(LogServerProjectile
		       , Log
		       , TEXT(
			       "Projectile hit something,this hit component name:%s,hit other component name:%s,other actor name :%s"
		       )
		       , HitComponent == nullptr ? TEXT("NULL") : *HitComponent->GetName()
		       , OtherComp == nullptr ? TEXT("NULL") : *OtherComp->GetName()
		       , OtherActor == nullptr ? TEXT("NULL") : *OtherActor->GetName());
		if (OtherActor && OtherActor->GetClass()->IsChildOf(AINSCharacter::StaticClass()))
		{
			AINSCharacter* HitCharacter = CastChecked<AINSCharacter>(OtherActor);
			FPointDamageEvent PointDamageEvent;
			PointDamageEvent.HitInfo = Hit;
			PointDamageEvent.Damage = OwnerWeapon->GetWeaponBaseDamage();
			PointDamageEvent.DamageTypeClass = UINSDamageType_Projectile::StaticClass();
			PointDamageEvent.ShotDirection = this->GetVelocity().GetSafeNormal();
			HitCharacter->TakeDamage(PointDamageEvent.Damage, PointDamageEvent, OwnerWeapon->GetINSPlayerController(),
			                         this);
		}
		UE_LOG(LogServerProjectile, Log,
		       TEXT("Projectile%s Hit Happened,Will Force Send a Movement info to all Conected Clients"), *GetName());
		RepHitInfo.Location = FVector_NetQuantize10(Hit.Location);
		RepHitInfo.LinearVelocity = FVector_NetQuantize(GetVelocity());
		FRotator HitRotator = GetActorRotation();
		FRotator RepRotator(FRotator::CompressAxisToByte(HitRotator.Pitch),
		                    FRotator::CompressAxisToByte(HitRotator.Yaw),
		                    FRotator::CompressAxisToByte(HitRotator.Roll));
		RepHitInfo.Rotation = RepRotator;
		if (GetLocalRole() == ROLE_Authority && !IsNetMode(NM_DedicatedServer))
		{
			OnRep_RepHitInfo();
		}
		bIsProcessingHit = false;
		//CalAndSpawnPenetrateProjectile(Hit, GetVelocity());
		SetLifeSpan(0.1f);
		if (IsNetMode(NM_Standalone) || IsNetMode(NM_ListenServer))
		{
			if (VisualProjectile)
			{
				VisualProjectile->OnServerProjectileHit(Hit);
			}
		}
	}
}

void AINSProjectile_Server::CreateVisualProjectile()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		UE_LOG(LogServerProjectile, Log, TEXT("Server projectile running on dedicated server will not create visual projectiles"));
		return;
	}
	UClass* VisualProjectileClass = OwnerWeapon->GetVisualProjectileClass();
	if (!VisualProjectileClass)
	{
		UE_LOG(LogServerProjectile, Warning, TEXT("Server projectile failed to create visual projectile because missing visual projectile class config"));
		return;
	}
	FVector SpawnLoc;
	UE_LOG(LogServerProjectile, Log, TEXT("Creating visual projectile in locations:%s"), *SpawnLoc.ToString());
	OwnerWeapon->GetBarrelStartLoc(SpawnLoc);
	const FTransform SpawnTrans((ScanTraceHitLoc - SpawnLoc).ToOrientationRotator(), SpawnLoc, FVector::OneVector);
	VisualProjectile = GetWorld()->SpawnActorDeferred<AINSProjectile_Visual>(
		VisualProjectileClass, SpawnTrans, this, OwnerWeapon->GetOwnerCharacter(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (VisualProjectile)
	{
		VisualProjectile->SetServerProjectile(this);
		VisualProjectile->SetOwnerWeapon(OwnerWeapon);
		VisualProjectile->SetIsScanTraceProjectile(bScanTraceProjectile);
		VisualProjectile->SetScanTraceTime(ScanTraceMoveTime);
		VisualProjectile->SetMuzzleSpeed(OwnerWeapon->GetMuzzleSpeedValue());
		TArray<AActor*> MoveIgnoredActor;
		MoveIgnoredActor.Emplace(this);
		MoveIgnoredActor.Emplace(OwnerWeapon);
		MoveIgnoredActor.Emplace(OwnerWeapon->GetOwnerCharacter());
		VisualProjectile->SetMoveIgnoredActor(MoveIgnoredActor);
		//VisualProjectile->SetActorHiddenInGame(true);
		UGameplayStatics::FinishSpawningActor(VisualProjectile, SpawnTrans);
	}
	if (IsNetMode(NM_Standalone) || IsNetMode(NM_ListenServer))
	{
		VisualProjectile->GetProjectileMovementComponent()->EnableTick(false);
		VisualProjectile->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		// VisualProjectile->SetProjectileTick(false);
		// VisualProjectile->GetProjectileMovementComponent()->EnableTick(false);
	}
}

void AINSProjectile_Server::SendInitialReplication()
{
	UE_LOG(LogServerProjectile, Log, TEXT("start send a initial bunch replocation %s manually to relavant Clients"),
	       *GetName());
	UNetDriver* const NetDriver = GetNetDriver();
	if (NetDriver != nullptr && NetDriver->IsServer() && !IsPendingKillPending() && (ProjectileMovementComponent->
	                                                                                 Velocity.Size() >= 7500.0f))
	{
		NetDriver->ReplicationFrame++;
		for (int32 i = 0; i < NetDriver->ClientConnections.Num(); i++)
		{
			if (NetDriver->ClientConnections[i]->GetConnectionState() == USOCK_Open && NetDriver->ClientConnections[i]->
				PlayerController != nullptr && NetDriver->ClientConnections[i]->IsNetReady(false))
			{
				const AActor* ViewTarget = NetDriver->ClientConnections[i]->PlayerController->GetViewTarget();
				if (ViewTarget == nullptr)
				{
					ViewTarget = NetDriver->ClientConnections[i]->PlayerController;
				}
				FVector ViewLocation = ViewTarget->GetActorLocation();
				{
					FRotator ViewRotation = NetDriver->ClientConnections[i]->PlayerController->GetControlRotation();
					NetDriver->ClientConnections[i]->PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
				}
				// Workaround to skip deprecation warning where it calls the PlayerController version of this function
				if (IsNetRelevantFor(NetDriver->ClientConnections[i]->PlayerController, ViewTarget, ViewLocation))
				{
					UActorChannel* Channel = NetDriver->ClientConnections[i]->FindActorChannelRef(this);
					if (Channel == nullptr)
					{
						if (NetDriver->ClientConnections[i]->GetClientWorldPackageName() == GetWorld()->GetOutermost()->
						                                                                                GetFName() && NetDriver->ClientConnections[i]->ClientHasInitializedLevelFor(this))
						{
							Channel = Cast<UActorChannel>(
								NetDriver->ClientConnections[i]->CreateChannelByName(
									NAME_Actor, EChannelCreateFlags::OpenedLocally, INDEX_NONE));
							if (Channel != nullptr)
							{
								Channel->SetChannelActor(this, ESetChannelActorFlags::None);
							}
						}
					}
					if (Channel != nullptr && Channel->OpenPacketId.First == INDEX_NONE)
					{
						if (!Channel->bIsReplicatingActor)
						{
							GatherCurrentMovement();
							Channel->ReplicateActor();
							UE_LOG(LogServerProjectile, Log,
							       TEXT("send a initial bunch replocation %s manually to  all relavent Clients done"),
							       *GetName());
						}
					}
				}
			}
		}
	}
}

void AINSProjectile_Server::NotifyVisualProjectileHit()
{
	if (IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	if (VisualProjectile)
	{
		VisualProjectile->ReceiveServerProjectileHit();
	}
}

// Called every frame
void AINSProjectile_Server::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AINSProjectile_Server::SetHitLocation(FVector_NetQuantize10 NewHitLoc)
{
	ScanTraceHitLoc = NewHitLoc;
}

void AINSProjectile_Server::SetScanTraceTime(const float NewTime)
{
	Super::SetScanTraceTime(NewTime);
	if (HasAuthority())
	{
		OnRep_ScanTraceTime();
	}
}

void AINSProjectile_Server::SetIsScanTraceProjectile(const bool bScanTrace)
{
	Super::SetIsScanTraceProjectile(bScanTrace);
	if (!bScanTraceProjectile)
	{
		SetReplicatingMovement(true);
	}
}
