// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"
#include "INSLobbyCharacter.generated.h"

class UCameraComponent;

/**
 *  this is for client only.
 *  visual representation for character contend some preset simple animations
 */
UCLASS()
class INSURGENCY_API AINSLobbyCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Weapon")
	TSubclassOf<AINSWeaponBase> LobbyWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Weapon")
	AINSWeaponBase* LobbyWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "LobbyCamera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* LobbyCamera;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void CreateClientLobbyWeapon();

	virtual void UpdateAnimation();
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetCurrentLobbyWeapon(int32 WeaponId);

	UFUNCTION(BlueprintPure)
	virtual UAnimMontage* GetWeaponBasePose() const;

	UFUNCTION(BlueprintPure)
	virtual class AINSWeaponBase* GetLobbyWeapon() const { return LobbyWeapon; };
};
