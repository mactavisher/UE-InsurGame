// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSGamePlay/INSTeamInfo.h"
#include "INSPlayerController.generated.h"

class AINSWeaponBase;
class AINSPlayerStateBase;
class UAudioComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogAINSPlayerController, Log, All)

/**
 * Ins Type of player controller
 */
UCLASS()
class INSURGENCY_API AINSPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

protected:

	/** INS Character that possessed by this controller */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "INSPlayerCharacter")
		AINSPlayerCharacter* PossessedINSCharacter;

	/** store input value last frame */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
		FRotator LastPlayerInputRot;

	/** default weapon class  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		TSubclassOf<AINSWeaponBase> PlayerDefaultWeaponClass;

	/** locally,is controlled pawn currently fires a weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Action")
		uint8 bPlayerFiring : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ReloadWeapon")
		uint8 bAutoReload : 1;

	/** cached Player State of INS Type */
	UPROPERTY()
		AINSPlayerStateBase* INSPlayerState;

	/** default weapon instance */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_PlayerTeam, Category = "Default")
		AINSTeamInfo* PlayerTeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AmbientAudioComp", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* AmbientAudioComp;

	/** current weapon used by this player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		class AINSWeaponBase* CurrentWeapon;

protected:
	/** possess a character */
	virtual void OnPossess(APawn* InPawn) override;


	/** replication support */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	/** set up player input bindings  */
	virtual void SetupInputComponent()override;

	/** perform move right ,negative value will perform move left */
	virtual void MoveRight(float Value);

	/** perform move forward,negative value will move backward */
	virtual void MoveForward(float Value);

	/** crouch */
	virtual void Crouch();

	/** crouch */
	virtual void UnCrouch();

	/** equips the slot 1 item*/
	virtual void EquipSlotItem_1();

	/** equips the slot 1 item*/
	virtual void EquipSlotItem_2();
	
	/** equips the slot 1 item*/
	virtual void EquipSlotItem_3();
	
	/** equips the slot 1 item*/
	virtual void EquipSlotItem_4();
	
	/** equips the slot 1 item*/
	virtual void EquipSlotItem_5();
	
	/** equips the slot 1 item*/
	virtual void EquipSlotItem_6();

	/** Server,perform move right ,negative value will perform move left */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerMoveRight(float Value);

	/** Server,perform move forward,negative value will move backwards */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerMoveForward(float Value);

public:

	/** aim */
	UFUNCTION()
		virtual void AimWeapon();
	/** server,aim */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerAimWeapon();

	/** stop aim */
	UFUNCTION()
		virtual void StopAimWeapon();

	/** server,stop aim */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerStopAimWeapon();

	/** reload weapon */
	virtual void ReloadWeapon();

	/** server,reload weapon */
	UFUNCTION(Reliable, Server, WithValidation)
		virtual void ServerReloadWeapon();

	/** server, finish reload a weapon */
	UFUNCTION(Reliable, Server, WithValidation)
		virtual void ServerFinishReloadWeapon();

	/** fire */
	virtual void Fire();

	/** server,fire */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerFire();

	/** stop fire */
	virtual void StopFire();

	virtual void Sprint();

	virtual void Jump();

	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerSprint();

	virtual void StopSprint();

	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerStopSprint();

	/** server,stop fire */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerStopFire();

	/** switch fire mode */
	virtual void SwitchFireMode();

	/** inspect weapon */
	virtual void InspectWeapon();

	/** sync server state to inspect weapon */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerInspectWeapon();

	virtual void PickupWeapon(class AINSPickup_Weapon* NewWeaponPickup);

	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerPickupWeapon(class AINSPickup_Weapon* NewWeaponPickup);

	/** server,switch fire mode */
	UFUNCTION(Reliable, Server, WithValidation)
		virtual void ServerSwitchFireMode();

	virtual void BeginPlay()override;

	UFUNCTION()
		virtual void OnRep_PlayerTeam();

	/** look up and down */
	virtual void AddPitchInput(float Val)override;

	/** turn left and right */
	virtual void AddYawInput(float Val)override;

	/** equip a weapon */
	virtual void EquipWeapon(const uint8 SlotIndex);

	virtual void Tick(float DeltaSeconds)override;

	/** server,equip a weapon */
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerEquipWeapon(const uint8 SlotIndex);


	virtual void SetWeaponState(EWeaponState NewState);

	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerSetWeaponState(EWeaponState NewState);

	/**
	 * @Desc Receive a info when this player enters a pickup
	 * @Param PickupItem  Item that this player enters
	 */
	virtual void ReceiveEnterPickups(class AINSPickupBase* PickupItem);

	/**
	 * @Desc Receive a info when this player leaves a pickup
	 * @Param PickupItem  Item that this player leaves
	 */
	virtual void ReceiveLeavePickups(class AINSPickupBase* PickupItem);

	UPROPERTY()
		FTimerHandle CharacterRespawnTimer;

public:

	virtual void PlayerCauseDamage(const FTakeHitInfo& HitInfo);

public:
	/** return possessed character  */
	virtual AINSPlayerCharacter* GetINSPlayerCharacter();

	/** returns last rotation input */
	UFUNCTION(BlueprintCallable)
		virtual FRotator GetLastRotationInput()const { return LastPlayerInputRot; };

	/** return s default weapon */
	virtual UClass* GetGameModeRandomWeapon();

	/** return s default weapon */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual  void ServerGetGameModeRandomWeapon();

	/** check to see if a given actor is consider as my enemy ,controllers typically*/
	virtual bool IsEnemyFor(class AActor* Other);

	/** check to see if we see a enemy,will be ticked for owner client */
	virtual bool HasSeeEnemy();

	virtual AINSPlayerStateBase* GetINSPlayerState();


	virtual void ReceiveOverlapPickupItems(class AActor* PickupItems);

	virtual void ReceiveStartRespawn();

	UFUNCTION()
		virtual void RespawnPlayer();

	UFUNCTION()
		virtual void  OnPlayerMeshSetupFinished();

	/**
	 * @desc add the player score
	 * @param  Score   the score to add
	 */
	virtual void PlayerScore(int32 Score);

	/**
	 * rep_notify when player state is replicated to owner client noticed that player controllers DOSE NOT exist on simulated proxies,
	 * so this function will only be called on owner client or server
	 */
	virtual void OnRep_PlayerState()override;

	/**
	 * @desc set the player's belonging team
	 * @param NewTeam New Team to set for this Player
	 */
	virtual void SetPlayerTeam(class AINSTeamInfo* NewTeam);

	/**
	 * override function called when player state get initiated
	 * player team should not  be set via this,because player state is initiated before player's team set
	 */
	virtual void InitPlayerState()override;

	/**
	 * rep_notify when pawn is replicated to owner client noticed that player controllers DOSE NOT exist on simulated proxies,
	 * so this function will only be called on owner client or server
	 */
	virtual void OnRep_Pawn()override;

	/**
	 * @desc returns the player team info
	 */
	inline virtual class AINSTeamInfo* GetPlayerTeam();

	/**
	 * @desc set the player's current weapon
	 * @param NewWeapon New Weapon to set for this Player
	 */
	virtual void SetCurrentWeapon(class AINSWeaponBase* NewWeapon);

	/**
	 * @desc bind to subscribe current weapon events
	 */
	virtual void BindWeaponDelegate();

	/**
	 * @desc called when player current weapon has no ammo left
	 */
	virtual void OnWeaponAmmoLeftEmpty();

	/**
	 * @desc called when player weapon current clip is low
	 */
	virtual void OnWeaponClipAmmoLow();

	/**
	 * @desc called when player weapon current clip is empty
	 */
	virtual void OnWeaponClipEmpty(class AController* WeaponOwnerPlayer);

	virtual void OnCharacterDead();
};
