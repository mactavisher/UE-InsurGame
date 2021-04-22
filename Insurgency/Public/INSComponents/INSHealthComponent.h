// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INSCharacter/INSCharacter.h"
#include "INSHealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterShouldDieSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLowHealthSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReduceHealthSignature);
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INSURGENCY_API UINSHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UINSHealthComponent();

	/** character's default health value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
		uint8 DefaultHealth;

	/** character's maximum health value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "HealthComponent")
		uint8 MaximunHealth;

	/**low health percentage hit value when character is not in full health  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
		float LowHealthPercentage;

	/**low health percentage hit value when character is not in full health  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
		float TimeBeforeHealthRestore;

	/** event syntax when character should die */
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnCharacterShouldDieSignature OnCharacterShouldDie;

	/** event syntax when character is in low health state */
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnLowHealthSignature OnLowHealth;

	/** event syntax when character health value reduced */
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnReduceHealthSignature OnHealthReduced;

protected:
	/** current health value,changes in run time */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, Category = "HealthComponent")
		float CurrentHealth;

	UPROPERTY()
		FTimerHandle HealthRestoreTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Owner")
		class AINSCharacter* OwnerCharacter;


	//~begin AActorComponent interface
	/**
	 * @override  component begin play event
	 */
	virtual void BeginPlay()override;

	/**
	 * @override component tick logic here
	 * @params     DeltaTime              frame tick interval time              float
	 * @params     TickType               level tick type                       ELevelTick
	 * @params     ThisTickFunction       tick function                         FActorComponentTickFunction*
	 */
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;

	//~end AActorComponent interface

public:
	/**
	 * @desc getter for default health value
	 * @return  default health value              float
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
		virtual  float GetDefaultHealth()const { return CurrentHealth; }

	/**
	 * @desc getter for current health value
	 * @return current health value              float
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
		virtual  float GetCurrentHealth()const;

	/**
	 * @desc health reduce logic here
	 * @param ReduceAmount          how much health to reduce        float
	 */
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		virtual void OnTakingDamage(float ReduceAmount, class AActor* DamageCauser, class AController* DamageInstigator);

	/**
	 * @desc restore current health value
	 * @param  GenerateAmount       how much health to restore        float
	 */
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		virtual void ReGenerateHealth();

	/**
	 * @desc getter for low low health percentage
	 * @return  LowHealthPercentage     LowHealthPercentage        float
	 */
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		virtual float GetLowHealthPercentage()const { return LowHealthPercentage; }


	/**
	 * @desc getter for current health percentage
	 * @return  health percentage     owner character current health percentage        float
	 */
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		virtual float GetHealthPercentage();
	/**
	 * @override function support replication
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	/**
	 * @desc check to see if current health hit low health percentage
	 * @return true or false           is low health?                     bool
	 */
	virtual bool CheckIsLowHealth();

	/**
	 * enable this component tick
	 */
	virtual void EnableComponentTick();

	/**
	 * disable this component tick
	 */
	virtual void DisableComponentTick();

	UFUNCTION()
		virtual void OnStopRestoreHealth();

	inline virtual class AINSCharacter* GetOwnerCharacter() { return CastChecked<AINSCharacter>(GetOwner()); }
};
