// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Insurgency/Insurgency.h"
#include "INSItems/INSItems.h"
#include "INSWeaponBase.generated.h"

class USoundCue;
class UParticleSystem;
class UParticleSystemComponent;
class UINSWeaponMeshComponent;
class UParticleSystemComponent;
class UINSWeaponAssets;
class AINSItems;
class AINSCharacter;
class UCameraShake;
class AINSProjectile;
class AINSProjectileShell;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSWeapon, Log, All);

/** weapon ammo config data */
USTRUCT(BlueprintType)
struct FWeaponConfigData
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		int32 AmmoPerClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		float ZoomingInTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		float ZoomingOutTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		float MuzzleSpeed;

public:
	FWeaponConfigData()
		: AmmoPerClip(30)
		, MaxAmmo(AmmoPerClip * 3)
		, TimeBetweenShots(0.15f)
		, ZoomingInTime(0.15f)
		, ZoomingOutTime(0.1f)
		, BaseDamage(20.f)
		, MuzzleSpeed(40000.f)
	{
	}
	void InitWeaponConfig()
	{
		AmmoPerClip = AmmoPerClip;
		MaxAmmo = AmmoPerClip * 5;
		TimeBetweenShots = TimeBetweenShots;
	}
};

namespace WeaponAttachmentSlotName
{
	  const FName Muzzle(TEXT("Muzzle"));	    // Muzzle slot name
	  const FName Sight(TEXT("Sight"));		// Sight slot name
	  const FName UnderBarrel(TEXT("UnderBarrel"));// UnderBarrel slot name
	  const FName LeftRail(TEXT("LeftRail"));	// LeftRail slot name
	  const FName rightRail(TEXT("rightRail"));	// rightRail slot name
}

/** weapon attachment slot */
USTRUCT(BlueprintType)
struct FWeaponAttachmentSlot {

	GENERATED_USTRUCT_BODY()

		/**attachment class */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AttachmentClass")
		TSubclassOf<AActor> WeaponAttachementClass;

	/** attachment instance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttachmentInstance")
		class AActor* WeaponAttachmentInstance;

	/** attachment instance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Availablility")
		uint8 bIsAvailable : 1;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AttachmentType")
		EWeaponAttachmentType WeaponAttachmentType;

public:
	/** return the class of The weapon class Attachment that will be used in this Attachment Slot */
	class AActor* GetWeaponAttachmentInstance()const { return WeaponAttachmentInstance; }

	/** return  Attachment instance that is used in this Attachment Slot */
	UClass* GetWeaponAttachmentClass()const { return WeaponAttachementClass; }

	/** return the Attachment type of the attachment slot */
	EWeaponAttachmentType GetAttachmentType()const { return WeaponAttachmentType; }

	/** return the Attachment type of the attachment slot */
	void  SetAttachmentType(EWeaponAttachmentType NewType)
	{
		WeaponAttachmentType = NewType;
	}

	FORCEINLINE FWeaponAttachmentSlot()
		:WeaponAttachementClass(nullptr)
		, WeaponAttachmentInstance(nullptr)
		, bIsAvailable(false)
	{
	}
	FORCEINLINE FWeaponAttachmentSlot(EWeaponAttachmentType WeaponAttachmentType)
		:WeaponAttachementClass(nullptr)
		, WeaponAttachmentInstance(nullptr)
		, bIsAvailable(true)
		, WeaponAttachmentType(WeaponAttachmentType)
	{

	}
	FORCEINLINE FWeaponAttachmentSlot(EWeaponAttachmentType WeaponAttachmentType,bool IsAvailable)
		:WeaponAttachementClass(nullptr)
		, WeaponAttachmentInstance(nullptr)
		, bIsAvailable(IsAvailable)
		, WeaponAttachmentType(WeaponAttachmentType)
	{

	}
};


/**
 *
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFireSignature, bool, bHasForeGip, bool, bIsDry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponStartReloadSignature, bool, bHasForeGip, bool, bIsDry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponStarSwitchFireModeSignature, bool, bHasForeGip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponStartEquipSignature, bool, bHasForeGrip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponEnterIdleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponOutIdleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponStartAimSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponStopAimSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFinishReloadSignature);

UCLASS()
class INSURGENCY_API AINSWeaponBase : public AINSItems
{
	GENERATED_UCLASS_BODY()

	/** stores available fire modes to switch between */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		TArray<EWeaponFireMode> AvailableFireModes;

	/** current selected(active) fire mode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_CurrentFireMode, Category = "Config")
		EWeaponFireMode CurrentWeaponFireMode;

	/** current weapon state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_CurrentWeaponState, Category = "Config")
		EWeaponState CurrentWeaponState;

	/** rep counter to tell clients fire just happened and things will happen */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_WeaponFireCount, Category = "Config")
		uint8 RepWeaponFireCount;

	/** simulate fire muzzle particles effects */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_WeaponFireCount, Category = "Config")
		UParticleSystemComponent* WeaponParticleComp;

	/** stores weapon config data */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
		FWeaponConfigData WeaponConfigData;

	/** ammo count in a current clip */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_CurrentClipAmmo, Category = "Ammo")
		int32 CurrentClipAmmo;

	/** ammo left in pocket */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated, Category = "Ammo")
		int32 AmmoLeft;

	/** if enabled,fire will not consumes any ammo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
		uint8 bInfinitAmmo : 1;

	/** is this weapon equip a fore grip */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Ammo")
		uint8 bForeGripEquipt : 1;

	/** count fire shots when in a semi-auto mode for control taking  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Ammo")
		uint8 SemiAutoCurrentRoundCount;

	/** stores last fire time , used for validate if weapon can fire it's next shot */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
		float LastFireTime;

	/** how much time it's gonna take to finish aim weapon  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "States")
		float AimTime;

	/** if enable ,weapon will reload automatically when current clip ammo hit 0 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
		uint8 bEnableAutoReload : 1;

	/** if enable ,weapon will reload automatically when current clip ammo hit 0 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Ammo")
		uint8 bDryReload : 1;

	/** if enable ,weapon will reload automatically when current clip ammo hit 0 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_AimWeapon, Category = "Aim")
		uint8 bisAiming : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_Equipping, Category = "WeaponActions")
		uint8 bWantsToEquip : 1;

	/** modify weapon IK To adjust weapon position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IKControl")
		FVector AdjustADSHandsIK;

	/** mesh 1p */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh1PComp", meta = (AllowPrivateAccess = "true"))
		UINSWeaponMeshComponent* WeaponMesh1PComp;

	/** mesh 3p */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh3PComp", meta = (AllowPrivateAccess = "true"))
		UINSWeaponMeshComponent* WeaponMesh3PComp;

	/** fire sound 1p*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
		USoundCue* FireSound1P;

	/** fire sound 3p*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
		USoundCue* FireSound3P;

	/** fire Particle 1p*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
		UParticleSystem* FireParticle1P;

	/** fire Particle 3p*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
		UParticleSystem* FireParticle3P;

	/** camera shaking effect class when fires a shot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recoil")
		TSubclassOf<UCameraShake> FireCameraShakingClass;

	/** projectile class that be fired by this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile")
		TSubclassOf<AINSProjectile> ProjectileClass;

	/** projectile shell class that be eject by this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
		TSubclassOf<AINSProjectileShell> ProjectileShellClass;

	/** weapon assets data,contains animation montage ,etc */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponAssets")
		UINSWeaponAssets* WeaponAssetsptr;

	/** pawn that owns this weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_OwnerCharacter, Category = "OwnerCharacter")
		AINSCharacter* OwnerCharacter;

	/** Max weapon spread value*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSpread")
		float WeaponSpreadMax;

	/** spread interpolate speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSpread")
		float WeaponSpreadInterpSpeed;

	/** current used weapon Spread */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSpread")
		float CurrentWeaponSpread;

	/** current used weapon Spread */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WeaponSpread")
		float BaseWeaponSpread;

	/** current used weapon Spread */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSpread")
		float RecoilVerticallyFactor;

	/** current used weapon Spread */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSpread")
		float RecoilHorizontallyFactor;

	/** base hand IK position */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "IKControll")
		FVector BaseHandsIk;

   
	/** WeaponAttachment Slots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponAttachments")
	 TMap<FName,FWeaponAttachmentSlot> WeaponAttachementSlots;

#if WITH_EDITORONLY_DATA
	uint8 bShowDebugTrace : 1;
#endif

	/** ~~--------------------------------------------------------------
	   timer handles-------------------------------------------*/

	   /** muzzle light timer */
	UPROPERTY()
		FTimerHandle MuzzleLightTimerHandle;

	/** Weapon bored timer handle */
	UPROPERTY()
		FTimerHandle WeaponBoredTimerHandle;

	/** Weapon bored timer handle */
	UPROPERTY()
		FTimerHandle WeaponBurstTimerHandle;

	/** Weapon bored timer handle */
	UPROPERTY()
		FTimerHandle WeaponSemiAutoTimerHandle;

	/** destroy if this weapon is dropped and not picked up for a period of time */
	UPROPERTY()
		FTimerHandle DestroyWeaponTimer;

	/** destroy if this weapon is dropped and not picked up for a period of time */
	UPROPERTY()
		FTimerHandle ResetFireStateTimer;
public:

	//~ begin weapon delegate signatures

	/** weapon fires a single shot delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponFireSignature OnWeaponEachFire;

	/** weapon switches it's fire mode delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponStarSwitchFireModeSignature OnWeaponSwitchFireMode;

	/** weapon reload delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponStartReloadSignature OnWeaponStartReload;

	/** weapon start equip delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponStartEquipSignature OnWeaponStartEquip;

	/** character with weapon enters idle state delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponEnterIdleSignature OnWeaponEnterIdle;

	/** character with weapon leaves idle state delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponOutIdleSignature OnWeaponOutIdle;

	/** character start weapon aim delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponStartAimSignature OnWeaponAim;

	/** character stop weapon aim delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponStopAimSignature OnStopWeaponAim;

	/** weapon just finishes it's reloading delegate */
	UPROPERTY(BlueprintAssignable, Category = "WeaponEvents")
		FOnWeaponFinishReloadSignature OnFinishReloadWeapon;

	//~ end  weapon delegate signatures

protected:
	//~ begin Actor interface
	virtual void Tick(float DeltaTime)override;

	virtual void PostInitializeComponents()override;

	virtual void BeginPlay()override;
	//~ end actor interface

	/** play weapon fire effects */
	virtual void SimulateWeaponFireFX();

	/** cast projectile shell after each shoot */
	virtual void CastShell();

	/**perform a trace to detect hit actor and adjust projectile spawn rotation */
	virtual void SimulateScanTrace(FHitResult& Hit);

	/** adjust projectile spawn rotation to hit center of the screen */
	virtual void AdjustProjectileDir(FVector& OutDir);

	/** check if can enter fire state */
	virtual bool CheckCanFire();

	/** check if can enter reload  state */
	virtual bool CheckCanReload();

	/** handles semi-auto fire if available for this weapon */
	virtual void InternalHandleSemiAutoFire();

	/** handles full auto fire if available for this weapon */
	virtual void InternalHandleBurstFire();

	/** adjust projectile make them spread */
	virtual void AddWeaponSpread(FVector& OutSpreadDir, FVector& BaseDirection);

	/** calculate ammo after each reload finishes */
	virtual void CalculateAmmoAfterReload();

	/** server,consumes a bullet on each shot ,default values is  1 */
	virtual void ConsumeAmmo(int32 AmmoAmount = 1);

	/** replication support */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	/** owner client,play camera shake effect */
	virtual void OwnerPlayCameraShake();

	/** updates the weapon current spread value */
	virtual void UpdateWeaponSpread(float DeltaTimeSeconds);

	/** set this weapon state to idle */
	UFUNCTION()
		virtual void ResetFireState();

	/** simulate logic happens when fires weapon once */
	UFUNCTION()
		virtual void SimulateEachSingleShoot();

	/** Fire Rep notify */
	UFUNCTION()
		virtual void OnRep_WeaponFireCount();

	/** owner Rep notify */
	UFUNCTION()
		virtual void OnRep_OwnerCharacter();

	/** Fire Mode Rep notify */
	UFUNCTION()
		virtual void OnRep_CurrentFireMode();

	UFUNCTION()
		virtual void OnRep_Equipping();

	/** current clip ammo Rep notify ,only relevant to owner  */
	UFUNCTION()
		virtual void OnRep_CurrentClipAmmo();

	/** current weapon state notify */
	UFUNCTION()
		virtual void OnRep_CurrentWeaponState();

	/** aim weapon notify */
	UFUNCTION()
		virtual void OnRep_AimWeapon();


public:
	/** returns pawn owner of this weapon,returns null if none */
	virtual AINSCharacter* GetOwnerCharacter();

	/** start reload weapon */
	virtual void StartReloadWeapon();

	/** server start reload Weapon */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerStartReloadWeapon();

	/** stop weapon fire ,will clear any fire timers and reset weapon state */
	virtual void StopWeaponFire();

	/** set weapon back to idle state */
	virtual void SetWeaponReady();

	/**start equip this weapon  */
	virtual void StartEquipWeapon();

	/**server,start equip this weapon  */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerStartEquipWeapon();

	/**aim this weapon  */
	virtual void StartWeaponAim();

	/**stop aim  this weapon  */
	virtual void StopWeaponAim();

	/**check if can aim  */
	virtual bool CheckCanAim();

	/** recoil Vertically when player fires */
	virtual void UpdateRecoilVertically(float DeltaTimeSeconds, float RecoilAmount);

	/** recoil horizontally when player fires */
	virtual void UpdateRecoilHorizontally(float DeltaTimeSeconds, float RecoilAmount);

	/** convert weapon state enum to a String ,make it easier to read */
	virtual FString GetWeaponReadableCurrentState();

	/** finish equip this weapon */
	virtual void FinishEquippingWeapon();

	/** server,finish equip this weapon */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerFinishEquippingWeapon();

	/** things need to set after reload a weapon */
	virtual void FinishReloadWeapon();

	/** things need to set after reload a weapon */
	UFUNCTION(Unreliable, Server, WithValidation)
		virtual void ServerFinishReloadWeapon();

	/** check if the weapon's owner is a local player controller or the owner pawn is locally controlled */
	UFUNCTION(BlueprintCallable)
		virtual bool GetIsOwnerLocal();

	/** switch between available fire mode with this weapon */
	virtual void StartSwitchFireMode();

	/** things need to set after weapon fire mode switched */
	virtual void FinishSwitchFireMode();

	/** server,things need to set after weapon fire mode switched */
	UFUNCTION(Unreliable, Server, WithValidation)
		virtual void ServerFinishSwitchFireMode();

	/** fires weapon once */
	virtual void FireWeapon();

	virtual void InspectWeapon();

	virtual void PreInitializeComponents()override;

	virtual void InitWeaponAttachmentSlots();

	virtual void GetWeaponAttachmentSlotStruct(FName SlotName,FWeaponAttachmentSlot& OutWeaponAttachmentSlot);

	/** spawns a projectile */
	virtual void SpawnProjectile(FVector SpawnLoc, FVector SpawnDir, float TimeBetweenShots);

	/** server,spawns a projectile */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerSpawnProjectile(FVector SpawnLoc, FVector SpawnDir, float TimeBetweenShots);

	/** set owner pawn of this weapon */
	virtual void SetOwnerCharacter(class AINSCharacter* NewOwnerCharacter);

	/** set this weapon current state */
	virtual void SetWeaponState(EWeaponState NewWeaponState) { CurrentWeaponState = NewWeaponState; };

	inline virtual EWeaponState GetWeaponCurrentState()const { return CurrentWeaponState; }

	/** server,set this weapon current state */
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerSetWeaponState(EWeaponState NewWeaponState);

	/** return weapon assets ref */
	FORCEINLINE virtual UINSWeaponAssets* GetWeaponAssets()const { return WeaponAssetsptr; };

	/** return whether this weapon equip with a fore grip ,this will affect animation poses and recoil*/
	inline virtual bool GetIsWeaponHasForeGrip()const { return bForeGripEquipt; }

	inline virtual float GetWeaponCurrentSpread()const { return CurrentWeaponSpread; }

	virtual float GetWeaponAimTime()const { return AimTime; }

	inline virtual float GetRecoilVerticallyFactor()const { return RecoilVerticallyFactor; }

	inline virtual float GetRecoilHorizontallyFactor()const { return RecoilHorizontallyFactor; }

	inline virtual FVector GetBaseHandsIk()const { return BaseHandsIk; }

	virtual void SetBaseHandsIk(FVector NewBaseHandsIk) { BaseHandsIk = NewBaseHandsIk; }

	inline virtual FVector GetAdjustADSHandsIk()const { return AdjustADSHandsIK; }

	virtual void SetAdjustADSHandsIk(FVector NewIKPosition) { AdjustADSHandsIK = NewIKPosition; }
};
