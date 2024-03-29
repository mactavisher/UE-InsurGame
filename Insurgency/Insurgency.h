// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Insurgency.generated.h"

/** ~~--------------------------------------------------------------
	define INS World Surface types --------------------------------*/

#define SurfaceType_Default                                        SurfaceType_Default
#define SurfaceType_Dirt                                           SurfaceType1
#define SurfaceType_Concret                                        SurfaceType2
#define SurfaceType_ThinMetal                                      SurfaceType3
#define SurfaceType_ThickMetal                                     SurfaceType4
#define SurfaceType_Grass                                          SurfaceType5
#define SurfaceType_Glass                                          SurfaceType6
#define SurfaceType_Water                                          SurfaceType7
#define SurfaceType_Wood                                           SurfaceType8
#define SurfaceType_Alsphat                                        SurfaceType9
#define SurfaceType_Ruber                                          SurfaceType10
#define SurfaceType_Flesh                                          SurfaceType11
#define SurfaceType_Gravel                                         SurfaceType12
#define SurfaceType_Mud                                            SurfaceType13
#define SurfaceType_Can                                            SurfaceType14
#define SurfaceType_Fabric                                         SurfaceType15
#define SurfaceType_Plastic                                        SurfaceType16
#define SurfaceType_Carpet                                         SurfaceType17
#define SurfaceTYpe_TreeLog                                        SurfaceType18
#define SurfaceType_TreeLeaves                                     SurfaceType19
#define SurfaceType_Armor                                          SurfaceType20

#define ECC_Penetrate                                              ECC_EngineTraceChannel1
#define ECC_Projectile                                             ECC_GameTraceChannel1

/** ~~--------------------------------------------------------------
   define INS Game Types-------------------------------------------*/

/** specify a character' gender,can be assigned via blueprint */
UENUM(BlueprintType)
enum class ECharacterGender :uint8
{
	MALE UMETA(DisplayName = "male"),
	FEMALE UMETA(DisplayName = "female"),
};

/** specify a character' view mode,relative to the local player  */
UENUM(BlueprintType)
enum class EViewMode :uint8
{
	FPS UMETA(DisplayName = "FPS"),
	TPS UMETA(DisplayName = "TPS"),
};

/** specify a character' stance types */
UENUM(BlueprintType)
enum class ECharacterStance :uint8
{
	STAND,
	CROUCH,
	PRONE,
};

/** specify a weapon's fire mode */
UENUM(BlueprintType)
enum class EWeaponFireMode :uint8
{
	SINGLE,
	SEMIAUTO,
	FULLAUTO,
};

UENUM(BlueprintType)
enum class EItemType :uint8
{
	WEAPON UMETA(DisplayName = "Weapon"),
	AMMO UMETA(DisplayName = "AMMO"),
	WEAPONATTACHEMENT UMETA(DisplayName = "WeaponAttachment"),
	HEALTH UMETA(DisplayName = "Health"),
	ARMOR UMETA(DisplayName = "ARMOR"),
	NONE UMETA(DisplayName = "NONE"),
};

UENUM(BlueprintType)
enum class EWeaponType :uint8
{
	ASSULTRIFLE UMETA(DisplayName = "AssultRifle"),
	PISTOL UMETA(DisplayName = "Pistol"),
	SHOTGUN UMETA(DisplayName = "ShotGun"),
	SMG UMETA(DisplayName = "SMG"),
	SNIPER UMETA(DisplayName = "Snipper"),
	BOLTRIFLE UMETA(DisplayName = "BoltRifle"),
	NONE UMETA(DisplayName = "None"),
};

/** weapon current state enum */
UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	IDLE,
	RELOADIND,
	UNEQUIPED,
	EQUIPPING,
	FIREMODESWITCHING,
	UNEQUIPING,
	EQUIPED,
	NONE,
};

/** weapon current zoom state enum */
UENUM(BlueprintType)
enum class EZoomState :uint8
{
	ZOOMING,
	ZOOMED,
	ZOZMINGOUT,
	ZOMMEDOUT,
};

/** weapon Priority enum */
UENUM(BlueprintType)
enum class EWeaponBasePoseType :uint8
{
	ALTGRIP UMETA(DisplayName = "alt grip"),
	FOREGRIP UMETA(DisplayName = "fore grip"),
	DEFAULT UMETA(DisplayName = "custom default"),
};

UENUM(BlueprintType)
enum class ETeamType :uint8
{
	REBEL UMETA(DisplayName = "Rebel"),
	ALLIE UMETA(DisplayName = "Allie"),
	CORP UMETA(DisplayName = "coorperation"),
	NONE UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class EGameType :uint8
{
	PVP UMETA(DisplayName = "PVP"),
	PVE UMETA(DisplayName = "PVE"),
};

UENUM(BlueprintType)
enum class EVoiceType :uint8
{
	TAKE_DAMAGE UMETA(DisplayName = "TakeDamage"),
	DIE UMETA(DisplayName = "Die"),
	TAKE_TEAM_DAMAGE UMETA(DisplayName = "TakeTeamDamage"),
	KILL_PLAYER UMETA(DisplayName = "Kill other player"),
	RELOADING UMETA(DisplayName = "Reloading"),
	SPOT_ENEMY UMETA(DisplayName = "Spot Enemy"),
	THROW_GRANADE UMETA(DisplayName = "Throw Granade"),
	THROW_SMOKING UMETA(DisplayName = "Throw Smoking"),
	MA_NDOWN UMETA(DisplayName = "Man down"),
	CAUSE_FRIENDLY_DAMAGE UMETA(DisplayName = "cause friendly damage"),
	CAUSE_FRIENDLY_KILL UMETA(DisplayName = "cause friendly kill"),
	NONE UMETA(DisplayName="none"),
};

UENUM(BlueprintType)
enum class EWeaponAttachmentType :uint8
{
	SIGHT UMETA(DisplayName = "Sight"),
	UNDER_BARREL UMETA(DisplayName = "Under Barrel"),
	MUZZLE UMETA(DisplayName = "Muzzle"),
	LEFT_RAIL UMETA(DisplayName = "Left Rail"),
	RIGHT_RAIL UMETA(DisplayName = "Right Rail"),
	NONE UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class EDamageEventID :uint8
{
	FALLING UMETA(DisplayName = "falling"),
	SHOT UMETA(DisplayName = "weapon shot"),
	EXPLOSION UMETA(DisplayName = "explosion"),
};

namespace TeamName
{
	static const FName Rebel = FName(TEXT("Rebel")); // Rebel team name
	static const FName Allie = FName(TEXT("Allie")); // Allie Team name
}

/**
 * replicated hit info
 */
USTRUCT(BlueprintType)
struct FTakeHitInfo
{
	GENERATED_USTRUCT_BODY()

	/** indicate if this shot has shot a pawn or damageAble actor*/
	UPROPERTY()
	uint8 bValidShot:1;

	/** shot direction pitch, manually compressed and decompressed */
	UPROPERTY()
	uint8 ShotDirPitch;

	/** shot direction yaw, manually compressed and decompressed */
	UPROPERTY()
	uint8 ShotDirYaw;

	/** actor that actually cause this damage */
	UPROPERTY()
	class AActor* DamageCauser;

	/** pawn instigate this damage */
	UPROPERTY()
	class APawn* InstigatorPawn;

	/** actor that actually takes this damage */
	UPROPERTY()
	class AActor* Victim;

	/** the amount of damage actually applied,after game mode modify the damage */
	UPROPERTY()
	int32 Damage;

	/** the location of the hit (relative to Pawn center) */
	UPROPERTY()
	FVector_NetQuantize RelHitLocation;

	/** how much momentum was imparted */
	UPROPERTY()
	FVector_NetQuantize Momentum;

	/** the damage type we were hit with */
	UPROPERTY()
	TSubclassOf<UDamageType> DamageType;

	/** has this damage make the victim dead? */
	UPROPERTY()
	uint8 bVictimDead : 1;

	/** is victim already dead since last damage */
	UPROPERTY()
	uint8 bVictimAlreadyDead : 1;

	/** is this damage caused by team */
	UPROPERTY()
	uint8 bIsTeamDamage : 1;

	/** hit bone name */
	UPROPERTY()
	FName HitBoneName;

	/** the amount of damage actually applied,after game mode modify the damage */
	UPROPERTY()
	uint8 bIsDirtyData : 1;

	FTakeHitInfo()
		: bValidShot(false)
		  , ShotDirPitch(0)
		  , ShotDirYaw(0)
		  , DamageCauser(nullptr)
		  , InstigatorPawn(nullptr)
		  , Victim(nullptr)
		  , Damage(0)
		  , RelHitLocation(ForceInit)
		  , Momentum(ForceInit)
		  , DamageType(nullptr)
		  , bVictimDead(false)
		  , bVictimAlreadyDead(false)
		  , bIsTeamDamage(false)
		  , HitBoneName(NAME_None)
		  , bIsDirtyData(true)
	{
	}

public:
	void EnsureReplication()
	{
		bIsDirtyData = false;
	}
};


UENUM(BlueprintType)
enum class EGameModeEnum :uint8
{
	FREE_FOR_ALL,
	TEAM_DEATH_MATCH,
	CAPTURE_THE_FLAG,
};


UENUM(BlueprintType)
enum class EWeaponReloadType :uint8
{
	/** one reload, one round*/
	ONCE,

	/** cycle reload ,such like shot gun , bolt rifles*/
	CYCLE,

	/** none of above*/
	NONE,
};
