// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "INSAssets/INSStaticAnimData.h"
#include "INSComponents/INSInventoryComponent.h"
#include "Insurgency/Insurgency.h"
#include "INSCharacter.generated.h"

class AINSCharacter;
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
class UPhysicalAnimationComponent;
class UINSStaticAnimData;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSCharacter, Log, All);


USTRUCT(BlueprintType)
struct FLastHitStateInfo
{
	GENERATED_USTRUCT_BODY()

	/** indicates how long will our pawn considered be in hit state*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitStateTime;

	UPROPERTY()
	float CurrentHitStateLastTime;

	UPROPERTY()
	AActor* LastHitActor;

	UPROPERTY()
	uint8 bDead:1;

	UPROPERTY()
	float DeathTime;

	FLastHitStateInfo()
		: HitStateTime(10.f)
		  , CurrentHitStateLastTime(0.f)
		  , LastHitActor(nullptr)
		  , bDead(false)
		  , DeathTime(0.f)
	{
	}
};

/** mapped bone and damage modifier  */
USTRUCT(BlueprintType)
struct FBoneDamageModifier
{
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
	float GetBoneDamageModifier(const FName BoneName)
	{
		const float* const BoneDamageModifier = BoneMappedDamageModifier.Find(BoneName);
		const float BoneDamageRandomSeed = FMath::RandRange(1.0f, 1.5f);
		return BoneDamageModifier == nullptr ? 1.f * BoneDamageRandomSeed : *(BoneDamageModifier) * BoneDamageRandomSeed;
	}

	/**
	 * @Desc judge a damage is caused by head shot or not
	 * @Param BoneName  the bone name bean hit with
	 * @return 
	 */
	static bool GetIsHeadShot(const FName BoneName)
	{
		return BoneName.ToString().Contains("Head", ESearchCase::IgnoreCase);
	}
};

USTRUCT()
struct FPendingWeaponEquipEvent
{
	GENERATED_USTRUCT_BODY()
	/** indicate the event create time */
	UPROPERTY()
	float EventCreateTime;

	/** the actual weapon class to equip*/
	UPROPERTY()
	int32 ItemId;

	/** indicates if this event is currently active,if true,can't be overriden,has to wait until it finishes*/
	UPROPERTY()
	uint8 bIsEventActive:1;

	UPROPERTY()
	uint8 WeaponSlotIndex;

	FPendingWeaponEquipEvent()
		: EventCreateTime(0.f)
		  , ItemId(0)
		  , bIsEventActive(false)
		  , WeaponSlotIndex(static_cast<uint8>(255))
	{
	}

	void ResetEvent()
	{
		EventCreateTime = 0.f;
		ItemId = 0;
		bIsEventActive = false;
		WeaponSlotIndex = static_cast<uint8>(255);
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
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_Dead, Category = "States")
	uint8 bIsDead : 1;

	/** used to prevent any cosmetic death events to play twice after dead  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	uint8 bIsDeadDead : 1;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	uint8 bIsSuppressed : 1;

	/** cache take hit array */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
	TArray<FTakeHitInfo> CachedTakeHitArray;

	/** replicated take hit info  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_LastHitInfo)
	FTakeHitInfo LastHitInfo;

	/** game time in real seconds when this pawn dead */
	UPROPERTY()
	float DeathTime;


	/** current stance of this character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_CurrentStance, Category = "Stances")
	ECharacterStance CharacterCurrentStance;

	/** noise emitter comp, give the ability for the character to make noise */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PawnMakeNoiseComp", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* NoiseEmitterComp;

	/** noise emitter comp, give the ability for the character to make noise */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PawnMakeNoiseComp", meta = (AllowPrivateAccess = "true"))
	UPhysicalAnimationComponent* PhysicalAnimationComponent;

	/** blood particle spawned when taking hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	TArray<UParticleSystem*> BloodParticles;

	/** blood particle spawned when taking hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	TArray<UParticleSystem*> BloodFlowParticles;

	/** Decal materials. provide bullet holes visual on body*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TArray<UMaterialInterface*> BulletDecalMaterials;

	/** Decal materials. provide blood spray on buildings such as wall*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TArray<UMaterialInterface*> BloodSprayDecalMaterials;

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

	/** bone mapped damage modifier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BoneDamageModifier")
	FBoneDamageModifier BoneNameMappedDamageModifier;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float CurrentEyeHeight;

	/** Init value for Character damage immune */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	uint8 InitDamageImmuneTime;

	/** time left for character immune since spawn */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, ReplicatedUsing = "OnRep_DamageImmuneTime", Category = "Damage")
	uint8 DamageImmuneLeft;

	/** indicates if character is currently in spawn protection state */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	uint8 bDamageImmuneState : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="LastHitState")
	FLastHitStateInfo LastHitState;

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

	UPROPERTY()
	UINSStaticAnimData* CurrentAnimPtr;

	FPendingWeaponEquipEvent PendingWeaponEquipEvent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Debugs")
	uint8 bShowDebugTrace : 1;
#endif

protected:
	//~ begin AActor interface
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GatherCurrentMovement() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Tick(float DeltaTime) override;
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual bool ShouldTakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const override;
public:
	virtual float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	//~ end AActor interface

protected:
	//~ begin ACharacter interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void ApplyDamageMomentum(float DamageTaken, const FDamageEvent& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void BecomeViewTarget(APlayerController* PC) override;
public:
	virtual void Crouch(bool bClientSimulation /* = false */) override;
	virtual void UnCrouch(bool bClientSimulation /* = false */) override;
	virtual void Die();
	//~ end ACharacter interface

protected:
	/** handle take point damage event */
	UFUNCTION()
	virtual void HandleOnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
	                                     const class UDamageType* DamageType, AActor* DamageCauser);

	/** handle take any damage event */
	UFUNCTION()
	virtual void HandleOnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/** handle take radius damage event */
	UFUNCTION()
	virtual void HandleOnTakeRadiusDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, FHitResult HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

	/** Cast Blood decal on static building when take damage */
	virtual void CastBloodDecal(FVector HitLocation, FVector HitDir);

	virtual void TossCurrentWeapon();

	virtual void CreateAndEquipItem(int32 ItemId, const uint8 InventorySlotIndex);

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void ServerCreateAndEquipItem(int32 ItemId, const uint8 InventorySlotIndex);

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

	/** Current Weapon Callback Rep notify*/
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
	/** return current weapon instance used by this character */
	UFUNCTION(BlueprintCallable)
	virtual class AINSWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	/**
	 * @desc get the bone mapped damage modifier struct data
	 *
	 * @param OutBoneDamageModifier
	 *        out put the BoneDamageModifier
	 */
	virtual void GetBoneDamageModifierStruct(FBoneDamageModifier& OutBoneDamageModifier)
	{
		OutBoneDamageModifier = BoneNameMappedDamageModifier;
	}

	/** return this character's health comp */
	FORCEINLINE virtual UINSHealthComponent* GetCharacterHealthComp() const { return CharacterHealthComp; }

	/** return ins version of character movement comp */
	FORCEINLINE virtual UINSCharacterMovementComponent* GetINSCharacterMovement() const { return INSCharacterMovementComp; };

	/** returns the character's audio comp */
	FORCEINLINE virtual class UINSCharacterAudioComponent* GetCharacterAudioComp() const { return CharacterAudioComp; }

	/** returns if this character is currently sprinting */
	virtual bool GetIsSprint() const { return bIsSprint; }

	/** returns if this character is currently crouched */
	virtual bool GetIsCrouched() const { return bIsCrouched; }

	/** return the character's current health value */
	inline virtual float GetCharacterCurrentHealth() const;

	/** handles Reload request from player  */
	virtual void HandleWeaponReloadRequest();

	/**
	 * On Weapon Collide
	 */
	virtual void OnWeaponCollide(const FHitResult& Hit);

	/**
	 * @desc  ticks the last hit state last time
	 * @param DeltaTime world delta time in seconds
	 */
	virtual void TickHitStateTime(const float DeltaTime);

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

	/** handles single ammo insert request ,usually request by anim notify*/
	virtual void HandleSingleAmmoInsertRequest();

	virtual void HandleFinishReloadingRequest();

	virtual void HandleFinishUnEquipWeaponRequest();

	virtual void HandleItemFinishEquipRequest();

	/** return this character is dead or not */
	virtual bool GetIsDead() const { return bIsDead; };

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

	/** handles jump request from player*/
	virtual void HandleJumpRequest();

	/** Handles Weapon Equip Request */
	virtual void HandleItemEquipRequest(const int32 NextItemId, const uint8 SlotIndex);

	/** Handles Weapon UnEquip Request */
	virtual void HandleItemFinishUnEquipRequest();
	virtual void UnEquipItem();
	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void ServerUnEquipItem();
	virtual void FinishUnEquipItem();
	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void ServerFinishUnEquipItem();

	/** callback when character crouched or un-crouched */
	virtual void OnRep_IsCrouched() override;

	/** spawns a weapon pick up  */
	virtual void SpawnWeaponPickup();

	/**
	* @desc set hands ik x location when character aim
	* @param     Value   hands ik x location value to set
	*/
	virtual void SetAimHandsXLocation(const float Value);

	virtual void SetWeaponBasePoseType(const EWeaponBasePoseType NewType);

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

	/**
	 * @desc set is character is suppressed
	 * @param     InState   InState to set
	 */
	virtual void SetIsSuppressed(bool InState) { bIsSuppressed = InState; }


	/** return is this character is suppressed by environment */
	virtual bool GetIsSuppressed() const;

	/** return is this character is in low health state */
	virtual bool GetIsLowHealth() const;

	/** get whether is character is in damage immune state */
	virtual bool GetIsDamageImmune() const { return bDamageImmuneState; }

	/** get character's damage immune time left */
	virtual uint8 GetDamageImmuneTimeLeft() const { return DamageImmuneLeft; }

	/** return is this character is in aiming state */
	virtual bool GetIsAiming() const { return bIsAiming; }

	/** return is this character is in aiming state */
	inline bool GetIsCharacterMoving() const;

	/** called when this character enters idle state, mainly used by Player characters,empty impl by default */
	virtual void OnEnterIdleState()
	{
	};

	/** called when this character out idle state, mainly used by Player characters,empty impl by default */
	virtual void OnOutIdleState()
	{
	};

	/** called when this character enters bored state, mainly used by Player characters,empty impl by default */
	virtual void OnEnterBoredState()
	{
	};

	/** called when this character enters bored state, mainly used by Player characters,empty impl by default */
	virtual void OnOutBoredState()
	{
	};

	/** on low health  */
	virtual void OnLowHealth();

	/** returns the current health of this character */
	virtual float GetCurrentHealth() const;

	virtual void ApplyPhysicAnimation();

	/** called when this character damages other character */
	virtual void OnCauseDamage(const FTakeHitInfo& HitInfo);

	/**
	 * @desc Set the hit state
	 * @param LastHitActor   the actual actor hit us last time
	 */
	virtual void SetLastHitStateInfo(class AActor* LastHitActor);

	/** returns if this character is in hit state */
	virtual bool GetIsInHitState() const { return LastHitState.CurrentHitStateLastTime > 0.f; };

	/**
	* @desc Set the Animation data reference,noticed this will also update the animation data referenced by anim instance
	* @param AnimData the animation data to set
	*/
	virtual void SetCurrentAnimData(UINSStaticAnimData* AnimData);

	/** returns if this pawn is a bot ,not player controlled */
	virtual bool GetIsABot();

	/** returns the death time for this character */
	virtual float GetCharacterDeathTime() const { return DeathTime; }

	/** checks if the character is already dead to make sure  the death base event will only executed once*/
	virtual bool GetCharacterIsAlreadyDead();

	/** local client check this distance to the other location before spawn some FX*/
	virtual float CheckDistance(const FVector OtherLocation);

	virtual void SetupPendingWeaponEquipEvent(const int32 ItemId, const uint8 ItemSlotIdx);

	virtual FPendingWeaponEquipEvent& GetPendingEquipEvent();

	virtual bool GetIsDeadDead() const { return bIsDeadDead; }

	virtual float PlayWeaponReloadAnim() { return 0.f; };

	virtual float PlayWeaponEquipAnim() { return 0.f; };

	virtual float PlayWeaponUnEquipAnim() { return 0.f; };

	virtual float PlayWeaponSwitchFireModeAnim() { return 0.f; };

	virtual float PlayFireAnim() { return 0.f; };

	virtual void UpdateAnimationData(class AINSItems* InItemRef);

	virtual bool CheckCharacterIsReady();

	virtual void OnShotFired()
	{
	};

	virtual void OnReloadFinished()
	{
	};

	virtual void ReceiveInventoryInitialized();

	virtual void ReceiveClipAmmoEmpty();

	virtual void ReceiveSetupWeaponAttachment();
};
