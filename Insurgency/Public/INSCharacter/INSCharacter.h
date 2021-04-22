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
class UPawnNoiseEmitterComponent;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSCharacter, Log, All);


/** mapped bone and damage modifier  */
USTRUCT(BlueprintType)
struct FBoneDamageModifier {
	GENERATED_USTRUCT_BODY()

protected:
	/** collection of bone mapped damage modifier,could be assigned via blueprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BoneDamageMap")
		TMap<FName, float> BoneMappedDamageModifier;
public:
	/**
	 * @Desc   convenient bone mapped damage modifier querier
	 *
	 * @params BoneName the bone name to query the modifier
	 *         if not found , will return 1.f,which means apply original damage,
	 *         else will return the modifier with some random seed add to it
	 * @Return the damage modifier
	 */
	float GetBoneDamageModifier(FName BoneName)
	{
		float* BoneDamageModifier = BoneMappedDamageModifier.Find(BoneName);
		float  BoneDamageRandomSeed = FMath::RandRange(1.0f, 1.5f);
		return BoneDamageModifier == nullptr ? 1.f * BoneDamageRandomSeed : *(BoneDamageModifier)*BoneDamageRandomSeed;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartSprintSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopSprintSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterMeshSetupFinishedSignature);

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
	UPROPERTY()
		TArray<FTakeHitInfo> CachedTakeHitArray;

	/**  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_WantsToSwitchFireMode, Category = "WeaponActions")
		uint8 bWantsToSwitchFireMode : 1;

	/** replicated take hit info  */
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_LastHitInfo)
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FallingDamage")
		float FatalFallingSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FallingDamage")
		UCurveFloat* FallingDamageCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FallingDamage")
		uint8 bEnableFallingDamage : 1;

	/** bone mapped damage modifier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BoneDamageModifier")
		FBoneDamageModifier  BoneNameMappedDamageModifier;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Camera")
		float CurrentEyeHeight;

	/** Init value for Character damage immune */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		uint8 InitDamageImmuneTime;

	/** time left for character immune since spawn */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, ReplicatedUsing = "OnRep_DamageImmuneTime", Category = "Damage")
		uint8 DamageImmuneLeft;

	/** indicates if character is currently in spawn protection mode */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		uint8 bDamageImmuneState : 1;

public:
	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
		FOnStartSprintSignature OnStartSprint;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
		FOnStartSprintSignature OnStopSprint;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
		FCharacterMeshSetupFinishedSignature CharacterSetupFinished;

	/** Decal materials. provide blood spray on buildings such as wall*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponRef")
		TSubclassOf<AINSPickup_Weapon> WeaponPickupClass;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Debugs")
		uint8 bShowDebugTrace : 1;
#endif


protected:
	//~ begin AActor interface
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents()override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	virtual void GatherCurrentMovement()override;
	virtual void OnRep_ReplicatedMovement()override;
	virtual void Tick(float DeltaTime) override;
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)const override;
public:
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)override;
	//~ end AActor interface

protected:
	//~ begin ACharacter interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)override;
	virtual void Landed(const FHitResult& Hit)override;
	//~ end ACharacter interface

	/** handle take point damage event */
	UFUNCTION()
		virtual void HandleOnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	/** handle take any damage event */
	UFUNCTION()
		virtual void HandleOnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/** handle take radius damage event */
	UFUNCTION()
		virtual void HandleOnTakeRadiusDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, FHitResult HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

	virtual void GatherTakeHitInfo(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	/** Cast Blood decal on static building when take damage */
	virtual void CastBloodDecal(FVector HitLocation, FVector HitDir);

	/** ~~--------------------------------------------------------------
	   Rep callbacks-------------------------------------------*/

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


	/** Damage Immune time Replicated call back on clients */
	UFUNCTION()
		virtual void OnRep_DamageImmuneTime();

	/** ~~--------------------------------------------------------------
		Timer Callbacks------------------------------------------------*/

		/** fired by a timer to count down the damage Immune state */
	UFUNCTION()
		virtual void TickDamageImmune();

	/** ~~--------------------------------------------------------------
		Timers---------------------------------------------------------*/

		/** timer ticks the damage Immune state  */
	UPROPERTY()
		FTimerHandle DamageImmuneTimer;

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
	FORCEINLINE virtual UINSCharacterMovementComponent* GetINSCharacterMovement()const { return INSCharacterMovementComp; };

	/** returns the character's audio comp */
	FORCEINLINE virtual class UINSCharacterAudioComponent* GetCharacterAudioComp()const { return CharacterAudioComp; }

	/** returns if this character is currently sprinting */
	inline virtual bool GetIsSprint()const { return bIsSprint; }

	/** returns if this character is currently crouched */
	inline virtual bool GetIsCrouched()const { return bIsCrouched; }

	/** return the character's current health value */
	inline virtual float GetCharacterCurrentHealth()const;

	/** handles Reload request from player  */
	virtual void HandleWeaponRealoadRequest();

	virtual void OnWeaponCollide(const FHitResult& Hit);

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

	/** handles sprint request from player*/
	virtual void HandleStartSprintRequest();

	/** handles stop sprint request from player*/
	virtual void HandleStopSprintRequest();

	/** callback when character crouched or un-crouched */
	virtual void OnRep_IsCrouched()override;

	/** spawns a weapon pick up  */
	virtual void SpawnWeaponPickup();

	virtual void Crouch(bool bClientSimulation /* = false */)override;

	virtual void UnCrouch(bool bClientSimulation /* = false */)override;

	/**
	 * @desc set is character is in a Aim State
	 * @param     NewAimState   NewAimState to set
	 */
	virtual void SetCharacterAiming(bool NewAimState);

	/**
	 * @desc set the character's current using weapon
	 * @param     NewWeapon   NewWeapon to set
	 */
	virtual void SetCurrentWeapon(class AINSWeaponBase* NewWeapon);

	/** return is this character is suppressed by environment */
	virtual bool GetIsSuppressed()const;
	/**
	 * @desc set is character is suppressed
	 * @param     InState   InState to set
	 */
	virtual void SetIsSuppressed(bool InState) { bIsSuppressed = InState; }

	virtual void BecomeViewTarget(APlayerController* PC)override;

	virtual bool GetIsLowHealth()const;

	/**
	 * @Desc get whether is character is in damage immune state
	 * @Return Is in Immune state bool
	 */
	virtual bool GetIsDamageImmune()const { return bDamageImmuneState; }

	/**
	 * @Desc get character's damage immune time left
	 * @Return damage immune time left
	 */
	virtual uint8 GetDamageImmuneTimeLeft()const { return DamageImmuneLeft; }

	UFUNCTION()
		virtual void OnDeath();

	inline virtual bool GetIsAiming()const { return bIsAiming; }



	inline bool GetIsCharacterMoving()const;
};
