// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Insurgency/Insurgency.h"
#include "INSItems/INSItems.h"
#include "INSItems/INSWeaponAttachments/INSWeaponAttachment.h"
#include "INSWeaponBase.generated.h"

class USoundCue;
class UParticleSystem;
class UParticleSystemComponent;
class UINSWeaponMeshComponent;
class UParticleSystemComponent;
class UINSWeaponAssets;
class AINSItems;
class AINSCharacter;
class AINSProjectile;
class AINSProjectileShell;
class UINSStaticAnimData;
class UINSWeaponFireHandler;
class UINSCrossHairBase;

INSURGENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogINSWeapon, Log, All);

/** weapon property config data */
USTRUCT(BlueprintType)
struct FWeaponConfigData
{
	GENERATED_USTRUCT_BODY()

	/** the maximum ammo can be hold in a single clip */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AmmoPerClip;

	/**the max ammo can carry with this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 MaxAmmo;

	/** fire interval */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TimeBetweenShots;

	/** time spent used to zoom in */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ZoomingInTime;

	/** time spent used to zoom out */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ZoomingOutTime;

	/** base damage of this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BaseDamage;

	/** muzzle speed , used to init projectile initial velocity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MuzzleSpeed;

	/** valid shoot range for this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ScanTraceRange;

	FWeaponConfigData()
		: AmmoPerClip(30)
		  , MaxAmmo(AmmoPerClip * 3)
		  , TimeBetweenShots(0.15f)
		  , ZoomingInTime(0.15f)
		  , ZoomingOutTime(0.15f)
		  , BaseDamage(20.f)
		  , MuzzleSpeed(40000.f)
		  , ScanTraceRange(2500.f)
	{
	}

	void ForceInitWeaponConfig()
	{
		AmmoPerClip = AmmoPerClip;
		MaxAmmo = AmmoPerClip * 10;
		TimeBetweenShots = TimeBetweenShots;
	}
};

USTRUCT(BlueprintType)
struct FWeaponSpreadData
{
	GENERATED_USTRUCT_BODY()

	friend class AINSWeaponBase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DefaultWeaponSpread;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float WeaponSpreadMax;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float WeaponSpreadMin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SpreadIncrementByShot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SpreadDecrement;

	/**current weapon spread value */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentWeaponSpread;

	/**current weapon spread value */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MovementSpreadScalingFactor;

	FWeaponSpreadData()
		: DefaultWeaponSpread(3.f)
		  , WeaponSpreadMax(10.f)
		  , WeaponSpreadMin(3.0)
		  , SpreadIncrementByShot(4.f)
		  , SpreadDecrement(0.5f)
		  , CurrentWeaponSpread(DefaultWeaponSpread)
		  , MovementSpreadScalingFactor(10.f)
	{
	}
};

UENUM(BlueprintType)
enum class EWeaponPendingEventType :uint8
{
	Fire,
	Reload,
	Equip,
	SwitchFireMode,
	UnEquip,
	None,
};

USTRUCT()
struct FWeaponPendingEvent
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	EWeaponPendingEventType EventType;

	UPROPERTY()
	float EventCreateTime;

	UPROPERTY()
	float DelayedExecuteTime;

	UPROPERTY()
	uint8 bIsValid : 1;

	UPROPERTY()
	float DelayedTimeElapsed;

	FWeaponPendingEvent():
		EventType(EWeaponPendingEventType::None)
		, EventCreateTime(0.f)
		, DelayedExecuteTime(0.f)
		, bIsValid(false)
		, DelayedTimeElapsed(0.f)
	{
	}

	void Reset()
	{
		EventType = EWeaponPendingEventType::None;
		EventCreateTime = 0.f;
		bIsValid = false;
		DelayedTimeElapsed = 0.f;
	}

	void Disable() { bIsValid = false; }

	void Activate() { bIsValid = true; }
};


namespace WeaponAttachmentSlotName
{
	const FName Muzzle(TEXT("Muzzle")); // Muzzle slot name
	const FName Sight(TEXT("Sight")); // Sight slot name
	const FName UnderBarrel(TEXT("UnderBarrel")); // UnderBarrel slot name
	const FName LeftRail(TEXT("LeftRail")); // LeftRail slot name
	const FName RightRail(TEXT("RightRail")); // rightRail slot name
}

/** weapon attachment slot */
USTRUCT(BlueprintType)
struct FWeaponAttachmentSlot
{
	GENERATED_USTRUCT_BODY()

	/**attachment class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AINSWeaponAttachment> WeaponAttachmentClass;

	/** attachment instance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AINSWeaponAttachment* WeaponAttachmentInstance;

	/** attachment instance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bIsAvailable : 1;

	/** Weapon attachment type */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	EWeaponAttachmentType WeaponAttachmentType;

public:
	/**
	 * @desc return the class of The weapon class Attachment that will be used in this Attachment Slot
	 */
	class AINSWeaponAttachment* GetWeaponAttachmentInstance() const { return WeaponAttachmentInstance; }

	/**
	 * @desc return  Attachment class  that is used in this Attachment Slot
	 */
	UClass* GetWeaponAttachmentClass() const { return WeaponAttachmentClass; }

	/**
	 * @desc return the Attachment type of the attachment slot
	 */
	EWeaponAttachmentType GetAttachmentType() const { return WeaponAttachmentType; }

	/**
	 * @desc return the Attachment type of the attachment slot
	   @param NewType  NewAttachmentType to set
	 */
	void SetAttachmentType(EWeaponAttachmentType NewType)
	{
		WeaponAttachmentType = NewType;
	}

	/**
	 * @desc init this struct by passing a attachment type
	 * @param WeaponAttachmentType set the attachment type of this attachment slot
	 */
	FWeaponAttachmentSlot(EWeaponAttachmentType WeaponAttachmentType)
		: WeaponAttachmentClass(nullptr)
		  , WeaponAttachmentInstance(nullptr)
		  , bIsAvailable(true)
		  , WeaponAttachmentType(WeaponAttachmentType)
	{
	}

	/**
	 * @desc   init this struct by passing a attachment type and Availability
	 * @param  WeaponAttachmentType  set the attachment type of this attachment slot
	 * @param  IsAvailable  set the Availability  of this attachment slot
	 */
	FWeaponAttachmentSlot(EWeaponAttachmentType WeaponAttachmentType, bool IsAvailable)
		: WeaponAttachmentClass(nullptr)
		  , WeaponAttachmentInstance(nullptr)
		  , bIsAvailable(IsAvailable)
		  , WeaponAttachmentType(WeaponAttachmentType)
	{
	}

	FWeaponAttachmentSlot():
		WeaponAttachmentClass(nullptr)
		, WeaponAttachmentInstance(nullptr)
		, bIsAvailable(true)
		, WeaponAttachmentType(EWeaponAttachmentType::NONE)
	{
	}
};

static const FName SightAlignerSocketName = FName(TEXT("SightAligner"));
UCLASS()
class INSURGENCY_API AINSWeaponBase : public AINSItems
{
	GENERATED_UCLASS_BODY()
	friend class UINSWeaponFireHandler;

	/** stores available fire modes to switch between */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FireMode")
	TArray<EWeaponFireMode> AvailableFireModes;

	/** weapon animation data */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UINSStaticAnimData> WeaponAnimationClass;

	/** weapon animation data */
	UPROPERTY()
	UINSStaticAnimData* WeaponAnimation;

	/** current selected active fire mode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_CurrentFireMode,
		Category = "FireMode")
	EWeaponFireMode CurrentWeaponFireMode;

	/** current weapon state */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_CurrentWeaponState,
		Category = "WeaponState")
	EWeaponState CurrentWeaponState;

	/** rep counter to tell clients fire just happened,mostly used for clients to play cosmetic events like fx */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_WeaponFireCount,
		Category = "WeaponState")
	uint8 RepWeaponFireCount;

	/** stores last fire time , used for validate if weapon can fire it's next shot */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponState")
	float LastFireTime;

	/** stores weapon config data */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "WeaponConfig")
	FWeaponConfigData WeaponConfigData;

	/** ammo count in a current clip */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_CurrentClipAmmo, Category = "Ammo")
	int32 CurrentClipAmmo;

	/** ammo left in pocket */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Ammo")
	int32 AmmoLeft;

#if WITH_EDITORONLY_DATA
	/** if enabled,fire will not consumes any ammo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	uint8 bInfinityAmmo : 1;
#endif

	/** if enable ,weapon will reload automatically when current clip ammo hit 0 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	uint8 bEnableAutoReload : 1;

	/** replicated dry reload state to client for reload animation play purpose */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Ammo")
	uint8 bDryReload : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_Equipping,
		Category = "Equipping")
	uint8 bWantsToEquip : 1;

	/** how much time it's gonna take to finish aim weapon  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aiming")
	float AimTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Aiming")
	uint8 ZoomedInEventTriggered : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Aiming")
	uint8 ZoomedOutEventTriggered : 1;

	/** if enable ,weapon will reload automatically when current clip ammo hit 0 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_AimWeapon, Category = "Aiming")
	uint8 bIsAimingWeapon : 1;

	/** current weapon state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming")
	EZoomState CurrentWeaponZoomState;

	/** mesh 1p */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh1PComp", meta = (AllowPrivateAccess = "true"))
	UINSWeaponMeshComponent* WeaponMesh1PComp;

	/** mesh 3p */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh3PComp", meta = (AllowPrivateAccess = "true"))
	UINSWeaponMeshComponent* WeaponMesh3PComp;

	/** mesh 3p */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponFireHandler", meta = (AllowPrivateAccess = "true"))
	UINSWeaponFireHandler* WeaponFireHandler;

	/** fire sound 1p*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	USoundCue* FireSound1P;

	/** fire sound 3p*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	USoundCue* FireSound3P;

	/** sound played when enters ads*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	USoundCue* ADSInSound;

	/** sound played when out ads*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	USoundCue* ADSOutSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	float ADSAlpha;

	/** fire Particle 1p*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* FireParticle1P;

	/** fire Particle 3p*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* FireParticle3P;

	/** projectile shell class that be eject by this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<AINSProjectileShell> ProjectileShellClass;

	/** simulate fire muzzle particles effects */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_WeaponFireCount,
		Category = "Effects")
	UParticleSystemComponent* WeaponParticleComp;

	/** projectile class that be fired by this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AINSProjectile> ProjectileClass;

	/** pawn that owns this weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_OwnerCharacter,
		Category = "OwnerCharacter")
	AINSCharacter* OwnerCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_WeaponBasePoseType,
		Category = "WeaponPose")
	EWeaponBasePoseType CurrentWeaponBasePoseType;

	/** current used weapon Spread */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recoil")
	float RecoilVerticallyFactor;

	/** current used weapon Spread */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recoil")
	float RecoilHorizontallyFactor;

	/** camera shaking effect class when fires a shot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recoil")
	TSubclassOf<UCameraShakeBase> FireCameraShakingClass;

	/** How big should the query probe sphere be (in unreal units),queries the weapon collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponCollision, meta = (editcondition = "bDoCollisionTest"))
	float ProbeSize;

	/** Max weapon spread value*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSpread")
	FWeaponSpreadData WeaponSpreadData;

	/** Cross hair class that be used by this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrossHair")
	TSubclassOf<UINSCrossHairBase> CrossHairClass;

	/** Cross hair instance */
	UPROPERTY()
	UINSCrossHairBase* CrossHair;

	/** WeaponAttachment Slots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponAttachments")
	TMap<FName, FWeaponAttachmentSlot> WeaponAttachmentSlots;

	/** reference from the inventory index */
	UPROPERTY()
	uint8 InventorySlotIndex;

	/** weapon type of this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_WeaponType,
		Category = "WeaponConfig")
	EWeaponType WeaponType;

	FActorTickFunction WeaponSpreadTickFunction;

#if WITH_EDITORONLY_DATA
	uint8 bShowDebugTrace : 1;
#endif


protected:
	//~ begin Actor interface
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~ end actor interface

	/**
	 *  @Desc play weapon fire effects
	 */
	virtual void SimulateWeaponFireFX();

	/**
	 * @Desc perform a trace from view center to Find what's under cross hair and produce that hit result when fire
	 *       
	 * @Return Hit Out the trace hit result
	 */
	virtual void FindCrossHairHit(FHitResult& Hit);

	/** check if can enter fire state */
	virtual bool CheckCanFire();

	/** check if can enter reload  state  */
	virtual bool CheckCanReload();

	/** called when weapon starts reload */
	virtual void OnWeaponStartReload();

	virtual void OnWeaponSwitchFireMode();

	/** called when equip weapon request has been called */
	virtual void OnWeaponStartEquip();

	/** update weapon collide info */
	virtual void UpdateWeaponCollide();

	/** called when weapon collide with environment */
	virtual void OnWeaponCollide(const FHitResult& CollideResult);

	/** calculate ammo after each reload finishes */
	virtual void CalculateAmmoAfterReload();

	/** server,consumes a bullet on each shot ,default values is  1 */
	virtual void ConsumeAmmo();

	/** owner client,play camera shake effect */
	virtual void OwnerPlayCameraShake();

	/** updates the weapon current spread value */
	virtual void UpdateWeaponSpread(float DeltaTimeSeconds);

	virtual void UpdateCharAnimationBasePoseType(EWeaponBasePoseType NewType);

	/** simulate logic happens when fires weapon once */
	UFUNCTION()
	virtual void SimulateEachSingleShoot();

	/** Fire Rep notify */
	UFUNCTION()
	virtual void OnRep_WeaponFireCount();

	UFUNCTION()
	virtual void OnRep_ScanTraceHit();

	/** owner Rep notify */
	UFUNCTION()
	virtual void OnRep_OwnerCharacter();

	/** Fire Mode Rep notify */
	UFUNCTION()
	virtual void OnRep_CurrentFireMode();

	/** Weapon Type Rep Notify */
	UFUNCTION()
	virtual void OnRep_WeaponType();

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

	UFUNCTION()
	virtual void OnRep_WeaponBasePoseType();

	virtual void OnRep_Owner() override;


public:
	/**
	 * returns pawn owner of this weapon,returns null if none
	 * the owner we set will typically be controller,but it's may be an AIController,
	 * so we provide a template here,return value might be null if class Type not compatible
	 */
	template <typename T>
	T* GetOwnerCharacter() const
	{
		return Cast<T>(OwnerCharacter);
	}

	virtual AINSCharacter* GetOwnerCharacter() { return OwnerCharacter; }
	/** start reload weapon */
	virtual void HandleWeaponReloadRequest();

	/** server start reload Weapon */
	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void ServerHandleWeaponReloadRequest();

	/** stop weapon fire ,will clear any fire timers and reset weapon state */
	virtual void StopWeaponFire();

	/** set weapon back to idle state */
	virtual void SetWeaponReady();

	virtual float GetScanTraceRange() const { return WeaponConfigData.ScanTraceRange; }

	virtual uint8 GetInventorySlotIndex() const { return InventorySlotIndex; }

	virtual void SetInventorySlotIndex(uint8 TargetSlot) { this->InventorySlotIndex = TargetSlot; }

	/**start equip this weapon  */
	virtual void StartEquipWeapon();

	/** update Weapon mesh visibility according to their local role */
	virtual void UpdateWeaponVisibility();

	/**server,start equip this weapon  */
	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void ServerStartEquipWeapon();

	/**aim this weapon  */
	virtual void StartWeaponAim();

	/**stop aim  this weapon  */
	virtual void StopWeaponAim();

	/**check if can aim  */
	virtual bool CheckCanAim();

	virtual float GetWeaponADSAlpha() const { return ADSAlpha; }

	virtual void SetOwner(AActor* NewOwner) override;

	/**
	 * @Desc Get the owner of INS type
	 * @Return INSPlayerController
	 */
	virtual class AINSPlayerController* GetINSPlayerController();

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

	/** switch between available fire mode with this weapon */
	virtual void StartSwitchFireMode();

	/** things need to set after weapon fire mode switched */
	virtual void FinishSwitchFireMode();
	UFUNCTION(Server, WithValidation, Unreliable)
	virtual void ServerFinishSwitchFireMode();

	/** being weapon fire */
	virtual void StartWeaponFire();

	/** client only, inspect current weapon*/
	virtual void InspectWeapon();

	/** check to see if we can switch fire mode*/
	virtual bool CheckCanSwitchFireMode();

	virtual void WeaponGoToIdleState();

	virtual bool GetIsWeaponInIdleState() { return CurrentWeaponState == EWeaponState::IDLE; }

	/**
	 * @desc init and create default attachment slot that this weapon will possess by default
	 */
	virtual void InitWeaponAttachmentSlots();

	/**
	 * @desc   get ads sight transform
	 * @params OutTransform calculate and produce the transform
	 */
	virtual void GetADSSightTransform(FTransform& OutTransform);

	/**
	 * @desc  return specific weapon attachment slot by name
	 * @param SlotName   the desired attachment Slot name
	 * @param OutWeaponAttachmentSlot produces the slot
	 */
	virtual void GetWeaponAttachmentSlotStruct(FName SlotName, FWeaponAttachmentSlot& OutWeaponAttachmentSlot);

	/**
	 * @desc  fire a projectile
	 * @param SpawnLoc   World Location to spawn to projectile
	 * @param SpawnDir   projectile spawn direction
	 */
	virtual void SpawnProjectile(FVector SpawnLoc, FVector SpawnDir);

	/**
	 * @desc  called by autonomous proxy clients to fire a projectile
	 * @param SpawnLoc   World Location to spawn to projectile
	 * @param SpawnDir   projectile spawn direction
	 */
	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void ServerSpawnProjectile(FVector SpawnLoc, FVector SpawnDir);

	/**
	 * @desc  set the character that own this weapon
	 * @param NewOwnerCharacter   the character to set
	 */
	virtual void SetOwnerCharacter(class AINSCharacter* NewOwnerCharacter);

	/**
	 * @Desc set this weapon current state
	 * @Param NewWeaponState new weapon state to set
	 */
	virtual void SetWeaponState(EWeaponState NewWeaponState);
	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void ServerSetWeaponState(EWeaponState NewWeaponState);

	/**
	 * @Desc returns the current weapon state
	 * @Return EWeaponState the weapon current state
	 */
	virtual EWeaponState GetWeaponCurrentState() const { return CurrentWeaponState; }

	FORCEINLINE virtual class UINSWeaponAnimInstance* GetWeapon1PAnimInstance();

	FORCEINLINE virtual class UINSWeaponAnimInstance* GetWeapon3pAnimInstance();

	virtual float GetWeaponCurrentSpread() const { return WeaponSpreadData.CurrentWeaponSpread; }

	FORCEINLINE virtual EWeaponFireMode GetCurrentWeaponFireMode() const { return CurrentWeaponFireMode; }

	virtual void SetWeaponCurrentFireMode(const EWeaponFireMode NewFireMode) { this->CurrentWeaponFireMode = NewFireMode; }

	virtual float GetWeaponAimTime() const { return AimTime; }

	virtual float GetRecoilVerticallyFactor() const { return RecoilVerticallyFactor; }

	virtual float GetRecoilHorizontallyFactor() const { return RecoilHorizontallyFactor; }

	virtual float GetTimeBetweenShots() const { return WeaponConfigData.TimeBetweenShots; }

	virtual void CheckAndEquipWeaponAttachment();

	/**
	 * @Desc adjust projectile spawn rotation to hit center of the screen
	 */
	virtual void GetFireDir(FVector& OutDir);

	/** adjust projectile make them spread */
	virtual void ApplyWeaponSpread(FVector& OutSpreadDir, const FVector& BaseDirection);

	/**
	 * @Desc  Gets the barrel Location
	 * @Param OutBarrelStartLoc produced Barrel Start Location
	 */
	virtual void GetBarrelStartLoc(FVector& OutBarrelStartLoc);

	/**
	 * returns the bullet muzzle velocity speed value
	 * @return WeaponConfigData.MuzzleSpeed    float
	 */
	virtual float GetMuzzleSpeedValue() const { return WeaponConfigData.MuzzleSpeed; }

	/**
	 * return the sight socket transform,in world space
	 */
	virtual FTransform GetSightsTransform() const;

	/** performs a line trace to check as a HitScan */
	virtual bool CheckScanTraceRange();

	/**
	 * @Desc Fire a shot by give shot location and rot
	 * @Param FireLoc target spawn location of this shot
	 * @param ShotRot target spawn Rotation of this shot
	 */
	virtual void FireShot(const FVector FireLoc, const FRotator ShotRot);

	/** executed cosmetic event when weapon fully zoomed out,clients only */
	virtual void OnZoomedOut();

	/** executed cosmetic event when weapon fully zoomed in,clients only */
	virtual void OnZoomedIn();

	/**
	 * called by HUD to draw to draw weapon crossHair
	 * @Param InCanvas  Canvas to draw Cross Hair on
	 * @Param DrawColor Draw color
	 */
	virtual void DrawCrossHair(class UCanvas* InCavas, const FLinearColor DrawColor);

	/**
	 * @Desc Update the ads status for client to execute cosmetic event
	 * @Param DeltaSeconds World DeltaTime
	 */
	virtual void UpdateADSStatus(const float DeltaSeconds);

	UFUNCTION(Server, Unreliable, WithValidation)
	virtual void ServerFireShot(FVector FireLoc, FRotator ShotRot);

	/**
	 * check to see if the weapon has a extra sight aligner
	 * @return true or false
	 */
	virtual bool IsSightAlignerExist() const;


	/**
	 * @Desc create the weapon CrossHair for client
	 * @return Weapon cross hair
	 */
	UFUNCTION(Client, WithValidation, Reliable)
	virtual void ClientCreateWeaponCrossHair();

	/**
	 * returns the weapon animation data
	 * @return WeaponAnimation UINSStaticAnimData
	 */
	virtual UINSStaticAnimData* GetWeaponAnimDataPtr() const { return WeaponAnimation; }

	/**
	 * returns the weapon state
	 * @return CurrentWeaponState EWeaponState
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual EWeaponState GetCurrentWeaponState() const { return CurrentWeaponState; }


	virtual EWeaponBasePoseType GetCurrentWeaponBasePose() const { return CurrentWeaponBasePoseType; }

	virtual void SetWeaponBasePoseType(const EWeaponBasePoseType NewPoseType) { CurrentWeaponBasePoseType = NewPoseType; }
};
