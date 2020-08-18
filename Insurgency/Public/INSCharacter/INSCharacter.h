// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Insurgency/Insurgency.h"
#include "INSCharacter.generated.h"

class UDamageType;
class UPawnNoiseEmitterComponent;
class UParticleSystem;
class AINSPlayerController;
class UINSCharSkeletalMeshComponent;
class AINSPlayerController;
class AINSWeaponBase;
class USoundCue;
class UINSCharacterMovementComponent;
class UINSHealthComponent;
class UINSCharacterAudioComponent;
class AINSPickup_Weapon;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSCharacter, Log, All);


/**
 * replicated hit info
 */
USTRUCT(BlueprintType)
struct FTakeHitInfo
{
	GENERATED_USTRUCT_BODY()

		/** the amount of damage actually applied,after game mode modify the damage */
		UPROPERTY()
		uint8 bIsDirtyData:1;

	/** the amount of damage actually applied,after game mode modify the damage */
	UPROPERTY()
		int32 Damage;

	/** initial damage should have applied before game mode to modify it's damage */
	UPROPERTY()
		int32 originalDamage;

	/** the location of the hit (relative to Pawn center) */
	UPROPERTY()
		FVector_NetQuantize RelHitLocation;

	/** how much momentum was imparted */
	UPROPERTY()
		FVector_NetQuantize Momentum;

	/** the damage type we were hit with */
	UPROPERTY()
		TSubclassOf<UDamageType> DamageType;

	/** shot direction pitch, manually compressed */
	UPROPERTY()
		uint8 ShotDirPitch;

	/** shot direction yaw, manually compressed */
	UPROPERTY()
		uint8 ShotDirYaw;

	/** actor that actually cause this damage */
	UPROPERTY()
		class AActor* DamageCauser;

	/** will this damage make the victim dead? */
	UPROPERTY()
		uint8 bVictimDead : 1;

	/** is victim already dead since last damage */
	UPROPERTY()
		uint8 bVictimAlreadyDead : 1;

	/** is this damage caused by team */
	UPROPERTY()
		uint8 bIsTeamDamage : 1;

	/** player that instigate this damage  */
	UPROPERTY()
		class AController* DamageInstigator;

	FTakeHitInfo()
		: bIsDirtyData(true)
		, Damage(0)
		, originalDamage(0)
		, RelHitLocation(ForceInit)
		, Momentum(ForceInit)
		, DamageType(NULL)
		, ShotDirPitch(0)
		, ShotDirYaw(0)
		, DamageCauser(NULL)
		, bVictimDead(false)
		, bVictimAlreadyDead(false)
		, bIsTeamDamage(false)
		, DamageInstigator(NULL)
	{
	}
};

/** mapped bone and damage modifier  */
USTRUCT(BlueprintType)
struct FBoneDamageModifier {
	GENERATED_USTRUCT_BODY()

protected:
		/** collection of bone mapped damage modifier,could be assigned via blueprint */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BoneDamageMap")
		TMap<FName, float> BoneMappedDamageModifier;

	/**
	 * convenient bone mapped damage modifier querier
	 *
	 * @params BoneName
	 *   the bone name to query the modifier
	 *
	 * if not found , will return 1.f,which means apply original damage,else will return the modifier with some random seed add to it
	 *
	 */
public:
	float GetBoneDamageModifier(FName BoneName)
	{
		float* BoneDamageModifier = nullptr;
		BoneDamageModifier = BoneMappedDamageModifier.Find(BoneName);
		if (BoneDamageModifier == nullptr)
		{
			return 1.f;
		}
		else
		{
			return *(BoneDamageModifier)*FMath::RandRange(1.f, 1.3f);
		}
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartSprintSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopSprintSignature);
UCLASS()
class INSURGENCY_API AINSCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

protected:

	/** is this character dead ? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_Dead, Category = "States")
		uint8 bIsDead : 1;

	/** is this character sprinting ? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_Sprint, Category = "Stances")
		uint8 bIsSprint : 1;

	/** is this character prone ? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_Prone, Category = "Stances")
		uint8 bIsProne : 1;

	/** is this character Aiming Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_Aim, Category = "States")
		uint8 bIsAiming : 1;

	/** is this character suppressed by environment */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "States")
		uint8 bIsSuppressed : 1;

	/** cache take hit array */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "States")
		TArray<FTakeHitInfo> CachedTakeHitArray;

	/**  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_WantsToSwitchFireMode, Category = "WeaponActions")
		uint8 bWantsToSwitchFireMode : 1;

	/** replicated take hit info  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_LastHitInfo, Category = "Damage")
		FTakeHitInfo LastHitInfo;

	/** current stance of this character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_CurrentStance, Category = "Stances")
		ECharacterStance CharacterCurrentStance;

	/** noise emitter comp, give the ability for the character to make noise */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PawnMakeNoiseComp", meta = (AllowPrivateAccess = "true"))
		UPawnNoiseEmitterComponent* NoiseEmmiterComp;

	/** blood particle spawned when taking hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
		TArray<UParticleSystem*> BloodParticles;

	/** Decal materials. provide bullet holes visual on body*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
		TArray<UMaterialInterface*> BulletDecalMaterials;

	/** Decal materials. provide blood spray on buildings such as wall*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
		TArray<UMaterialInterface*> BloodSprayDecalMaterials;

	/** Current Player controller that control this character */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerController")
		AINSPlayerController* CurrentPlayerController;

	/** Current Weapon that this character use */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_CurrentWeapon, Category = "WeaponRef")
		AINSWeaponBase* CurrentWeapon;

	/** body hit sound when receives damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
		USoundCue* BodyHitSound;

	/** Customized Character movement comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CharacterMovementComp", meta = (AllowPrivateAccess = "true"))
		UINSCharacterMovementComponent* INSCharacterMovementComp;

	/** Health comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CharacterHealthComp", meta = (AllowPrivateAccess = "true"))
		UINSHealthComponent* CharacterHealthComp;

	/** Audio comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CharacterAudioComp", meta = (AllowPrivateAccess = "true"))
		UINSCharacterAudioComponent* CharacterAudioComp;

	/** characters default eye height */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EyeHeight")
		float DefaultBaseEyeHeight;

	/** characters default eye height */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EyeHeight")
		float CurrentEyeHeight;

	/** bone mapped damage modifier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BoneDamageModifier")
		FBoneDamageModifier  BoneNameMappedDamageModifier;

public:
	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
		FOnStartSprintSignature OnStartSprint;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
		FOnStartSprintSignature OnStopSprint;

	/** Decal materials. provide blood spray on buildings such as wall*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponRef")
		TSubclassOf<AINSPickup_Weapon> WeaponPickupClass;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Debugs")
		uint8 bShowDebugTrace : 1;
#endif


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** initialize properties in c++ side */
	virtual void PostInitializeComponents()override;

	/** take momentum damage such as falling and knock by high speed things like cars */
	virtual void ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)override;

	/** handle take point damage event */
	UFUNCTION()
		virtual void HandleOnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	/** handle take any damage event */
	UFUNCTION()
		virtual void HandleOnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/** handle take radius damage event */
	UFUNCTION()
		virtual void HandleOnTakeRadiusDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, FHitResult HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

	/** replication support */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	/** landed */
	virtual void Landed(const FHitResult& Hit)override;

	/** Cast Blood decal on static building when take damage */
	virtual void CastBloodDecal(FVector HitLocation, FVector HitDir);

	/** things need to do when this character is dead  */
	UFUNCTION()
		virtual void OnRep_Dead();

	/** things need to do when clients receive this character's take hit event */
	UFUNCTION()
		virtual void OnRep_LastHitInfo();

	/** server+client ,call back function when this Character changes it's stance*/
	UFUNCTION()
		virtual void OnRep_CurrentStance();

	/** server+client ,call back function when this Character start to switch fire mode*/
	UFUNCTION()
		virtual void OnRep_WantsToSwitchFireMode();

	/** server+client ,call back function when this Character start to sprint*/
	UFUNCTION()
		virtual void OnRep_Sprint();

	/** server+client ,call back function when this Character start to aim*/
	UFUNCTION()
		virtual void OnRep_Aim();

	/** server+client ,call back function when this Character start to prone*/
	UFUNCTION()
		virtual void OnRep_Prone();

	/** server+client ,call back function when this Character equips a weapon*/
	UFUNCTION()
		virtual void OnRep_CurrentWeapon();


public:
	/** return this character is dead or not */
	inline virtual bool GetIsCharacterDead()const { return bIsDead; };

	/** return current weapon instance used by this character */
	UFUNCTION(BlueprintCallable)
		virtual class AINSWeaponBase* GetCurrentWeapon()const { return CurrentWeapon; }

	/**
	 * @desc get the bone mapped damage modifier struct data
	 *
	 * @param OutBoneDamageModifier
	 *        out put the BoneDamageModifier
	 */
	virtual void GetBoneDamageModifierStruct(FBoneDamageModifier& OutBoneDamageModifier) { OutBoneDamageModifier = BoneNameMappedDamageModifier; }

	/** return this character's health comp */
	FORCEINLINE virtual UINSHealthComponent* GetCharacterHealthComp()const { return CharacterHealthComp; }

	/** return ins version of character movement comp */
	FORCEINLINE virtual UINSCharacterMovementComponent* GetINSCharacterMovement();

	/** returns the character's audio comp */
	FORCEINLINE virtual class UINSCharacterAudioComponent* GetCharacterAudioComp()const { return CharacterAudioComp; }

	/** calls when this character receives any damage */
	virtual void ReceiveHit(AController* const InstigatorPlayer, AActor* const DamageCauser, const FDamageEvent& DamageEvent, const FHitResult& Hit, float DamageTaken);

	/** take damage ,if you call Super::TakeDamage(),the super function has broadcast take certain type of damage base on damageEnvent::clasId
	 *  such as point damageEvents,Radius damageEvents,otherwise you will need to broadcast those take damage event manually
	 */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)override;

	/** returns if this character is currently sprinting */
	inline virtual bool GetIsSprint()const { return bIsSprint; }

	/** returns if this character is currently crouched */
	inline virtual bool GetIsCrouched()const { return bIsCrouched; }

	/** handles Reload request from player  */
	virtual void HandleWeaponRealoadRequest();

	/** handles Aim request from player  */
	virtual void HandleAimWeaponRequest();

	/** handles Stop Aim request from player  */
	virtual void HandleStopAimWeaponRequest();

	/** handles Start fire request from player  */
	virtual void HandleFireRequest();

	/** handles stop fire request from player*/
	virtual void HandleStopFireRequest();

	/** handles equip weapons request from player */
	virtual void HandleEquipWeaponRequest();

	/** handles switch fire mode request from player */
	virtual void HandleSwitchFireModeRequest();

	/** handles move forward request from player,negative value means move backwards*/
	virtual void HandleMoveForwardRequest(float Value);

	/** handles move right request from player,negative value means move left*/
	virtual void HandleMoveRightRequest(float Value);

	/** handles crouch request from player*/
	virtual void HandleCrouchRequest();

	virtual void HandleStartSprintRequest();

	virtual void HandleStopSprintRequest();

	virtual void OnRep_IsCrouched()override;

	virtual void SpawnWeaponPickup();

	virtual void SetIsAiming(bool NewAimState) { this->bIsAiming = NewAimState; }

	virtual void SetCurrentWeapon(class AINSWeaponBase* NewWeapon);

	virtual bool GetIsSuppressed()const;

	virtual void SetIsSuppressed(bool InState) { bIsSuppressed = InState; }

	virtual void BecomeViewTarget(APlayerController* PC)override;

	UFUNCTION()
		virtual void OnDeath();

	inline virtual bool GetIsAiming()const { return bIsAiming; }

	inline bool GetIsCharacterMoving()const { return GetVelocity().Size() > 0.f; }

	virtual void KilledBy(class AController* PlayerKilledMe, class AACtor* ActorKilledMe);
};
