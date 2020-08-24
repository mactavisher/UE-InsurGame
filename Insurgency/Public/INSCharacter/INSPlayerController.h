// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "INSCharacter/INSPlayerCharacter.h"
#include "INSGamePlay/INSTeamInfo.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSPlayerController.generated.h"

class AINSWeaponBase;

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

	/** default weapon instance */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Default")
		AINSWeaponBase* DefaultWeapon;

	/** locally,is controlled pawn currently fires a weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Action")
		uint8 bPlayerFiring : 1;

protected:
	/** possess a character */
	virtual void OnPossess(APawn* InPawn) override;

	/** un-possess a character */
	virtual void OnUnPossess()override;

	/** set up player input bindings  */
	virtual void SetupInputComponent()override;

	/** perform move right ,negative value will perform move left */
	virtual void MoveRight(float Value);

	/** perform move forward,negative value will move backward */
	virtual void MoveForward(float Value);

	/** Server,perform move right ,negative value will perform move left */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerMoveRight(float Value);

	/** Server,perform move forward,negative value will move backwards */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerMoveForward(float Value);

	/** crouch */
	virtual void Crouch();

	/** server,crouch */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerCrouch();

	/** crouch */
	virtual void UnCrouch();

	/** server,crouch */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerUnCrouch();

	/** replication support */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
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

	UFUNCTION(Server,Unreliable,WithValidation)
	virtual void ServerSprint();

	virtual void StopSprint();

	UFUNCTION(Server,Unreliable,WithValidation)
	virtual void ServerStopSprint();

	/** server,stop fire */
	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerStopFire();

	/** switch fire mode */
	virtual void SwitchFireMode();

	/** inspect weapon */
	virtual void InspecWeapon();

	UFUNCTION(Server,Unreliable,WithValidation)
	virtual void ServerInspectWeapon();

	virtual void PickupWeapon(class AINSPickup_Weapon* NewWeaponPickup);

	UFUNCTION(Server, Unreliable, WithValidation)
		virtual void ServerPickupWeapon(class AINSPickup_Weapon* NewWeaponPickup);

	/** server,switch fire mode */
	UFUNCTION(Reliable, Server, WithValidation)
		virtual void ServerSwitchFireMode();

	virtual void BeginPlay()override;

	/** look up and down */
	virtual void AddPitchInput(float Val)override;

	/** turn left and right */
	virtual void AddYawInput(float Val)override;

	/** equip a weapon */
	virtual void EquipWeapon(class AINSWeaponBase* NewWeaponToEquip);

	virtual void Tick(float DeltaSeconds)override;

	/** server,equip a weapon */
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerEquipWeapon(class AINSWeaponBase* NewWeaponToEquip);

	virtual void ReceiveEnterPickups(class AINSItems_Pickup* PickupItem);

	virtual void ReceiveLeavePickups(class AINSItems_Pickup* PickupItem);

	UPROPERTY()
		FTimerHandle CharacterRespawnTimer;

public:
	/** owner client show a threaten indicator */
	UFUNCTION(Client, Unreliable, WithValidation)
		virtual void ClientShowThreaten();

public:
	/** return possessed character  */
	virtual AINSPlayerCharacter* GetINSPlayerCharacter();

	/** returns last rotation input */
	UFUNCTION(BlueprintCallable)
		virtual FRotator GetLastRotationInput()const { return LastPlayerInputRot; };

	/** return s default weapon */
	virtual class AINSWeaponBase* GetDefaultWeapon();

	/** check to see if a give actor is consider as my enemy ,controllers typically*/
	virtual bool IsEnemyFor(class AActor* Other);

	/** check to see if we see a enemy,will be ticked for owner client */
	virtual bool HasSeeEnemy();

	/** handle possessed Character death */
	virtual void OnCharacterDeath();


	virtual class AINSTeamInfo* GetMyTeamInfo();

	virtual void ReceiveOverlapPickupItems(class AActor* PickupItems);

	virtual void ReceiveStartRespawn();

	UFUNCTION()
		virtual void RespawnPlayer();

	virtual void PlayerScore(int32 Score);
};
