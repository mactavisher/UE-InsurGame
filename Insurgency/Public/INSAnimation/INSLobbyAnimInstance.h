// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "INSCharacter/INSLobbyCharacter.h"
#include "INSLobbyAnimInstance.generated.h"

class UAnimMontage;
class AINSWeaponBase;

/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSLobbyAnimInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="LobbyAnimation")
	TArray<UAnimMontage*> RelaxAnims;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="LobbyAnimation")
	UAnimMontage* WeaponBasePoseAnim;

	UPROPERTY()
	UAnimMontage* CurrentActiveRelaxAnim;

	UPROPERTY()
	AINSLobbyCharacter* OwnerLobbyCharacter;

	UPROPERTY()
	AINSWeaponBase* LobbyWeapon;
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void PlayWeaponBasePose();
	virtual void PlayRelaxAnim();
public:
	virtual void UpDateWeaponBasePoseAnim(UAnimMontage* NewAnim);
	virtual UAnimMontage* GetWeaponBasePose()const{return  WeaponBasePoseAnim;}

	virtual void SetLobbyWeapon(AINSWeaponBase* NewWeapon);
	virtual AINSWeaponBase* GetLobbyWeapon()const{return LobbyWeapon;}
};
