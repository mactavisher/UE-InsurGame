// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INSCharacter/INSCharacter.h"
#include "INSPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AINSWeaponBase;
class UINSCharSkeletalMeshComponent;
class UINSInventoryComponent;
class UINSFPAnimInstance;
class UINSTPAnimInstance;

USTRUCT(BlueprintType)
struct FDefaultPlayerMesh
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DefaultMesh1p")
	USkeletalMesh* Mesh1p;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DefaultMesh1p")
	USkeletalMesh* Mesh3p;
};

/**
 *   player controlled characters class
 */
UCLASS()
class INSURGENCY_API AINSPlayerCharacter : public AINSCharacter
{
	GENERATED_UCLASS_BODY()
protected:
	/** Player camera comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FirstPersonCamera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

	/** Player camera comp */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FirstPersonCamera", meta = (AllowPrivateAccess = "true"))
	UINSInventoryComponent* InventoryComp;

	/** player character's 1P mesh comp,only visible to owner player */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CharacterMesh")
	FDefaultPlayerMesh CTDefaultMesh;

	/** Player character's 3P mesh comp,only visible to non-owner player */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CharacterMesh")
	FDefaultPlayerMesh TerroristDefaultMesh;

	/** player character's 1P mesh comp,only visible to owner player */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	UINSCharSkeletalMeshComponent* CharacterMesh1P;

	/** Player character's 3P mesh comp,only visible to non-owner player */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "3PMesh")
	UINSCharSkeletalMeshComponent* CharacterMesh3P;

	/** Player character's 3P mesh comp,only visible to non-owner player */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "3PMesh")
	UINSCharSkeletalMeshComponent* CharacterMesh1P_Foot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, ReplicatedUsing=OnRep_TeamType, Category = "Team")
	ETeamType MyTeamType;

	/** crouched relative location of springArm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float SpringArmRelLocCrouched;

	FTimerHandle EquipDefaultWeaponHandle;


	/** Current Player controller that control this character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerController")
	AINSPlayerController* INSPlayerController;

	UPROPERTY()
	UINSFPAnimInstance* FPSAnimInstance;

	UPROPERTY()
	UINSTPAnimInstance* TPSAnimInstance;

	UPROPERTY()
	TArray<UINSCharacterAimInstance*> CachedAnimInstances;

	FActorTickFunction FirstEquipTickFunction;


protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual void OnCauseDamage(const FTakeHitInfo& HitInfo) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_CurrentWeapon() override;
	virtual void OnRep_Dead() override;
	virtual void OnRep_Aim() override;
	virtual void OnRep_IsCrouched() override;
	virtual void OnRep_Sprint() override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_LastHitInfo() override;
	virtual void Crouch(bool bClientSimulation) override;
	virtual void UnCrouch(bool bClientSimulation) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void SetAimHandsXLocation(const float Value) override;
	virtual void UpdateComponents();
	virtual void RegisterFirstEquipCheck();
	UFUNCTION()
	virtual void OnRep_TeamType();
	virtual void UpdateCharacterMesh1P(float DeltaTime);
	virtual void SetupAnimInstance();
	virtual void UpdateAnimationData(class AINSItems* InItemRef) override;


	/**
	 * @desc called when owner gets replicated and for controllers , 
	 *       this will only get called on server or autonomous_proxy clients
	 */
	virtual void OnRep_Owner() override;

	virtual void SetOwner(AActor* NewOwner) override;

	/**
	 * called when player controller gets replicated,this will only be called on Role_Athourity or Role_AutonomousProxy
	 */
	virtual void OnRep_Controller() override;

	UFUNCTION()
	virtual void UpdateCrouchEyeHeightSmoothly();

public:
	/** handles a friendly fire event */
	virtual void ReceiveFriendlyFire(class AINSPlayerController* InstigatorPlayer, float DamageTaken);

	/** return character camera comp */
	FORCEINLINE virtual UCameraComponent* GetPlayerCameraComp() const { return FirstPersonCamera; }

	virtual FTransform GetPlayerCameraTransform() const;

	/**
	 * @desc  get the camera socket transform in world space
	 *        this will usually access by roles that will take FPS view 
	 * 
	 * @Param OutCameraSocketTransform  produce the CameraSocketTransform  FTransform
	 */
	virtual void GetPlayerCameraSocketWorldTransform(FTransform& OutCameraSocketTransform);

	/** returns character's 3P mesh comp */
	FORCEINLINE UINSCharSkeletalMeshComponent* GetCharacter3PMesh() const { return CharacterMesh3P; }

	/** returns character's 1p mesh comp */
	FORCEINLINE UINSCharSkeletalMeshComponent* GetCharacter1PMesh() const { return CharacterMesh1P; }

	/** returns current equipped weapon of this character */
	virtual void SetCurrentWeapon(class AINSWeaponBase* NewWeapon) override;
	virtual void ReceiveSetupWeaponAttachment();

	/** handles a move forward request from player controller */
	virtual void HandleMoveForwardRequest(float Value) override;

	/** handles a move right request from player controller */
	virtual void HandleMoveRightRequest(float Value) override;

	/** handle s sprint request from player controller */
	virtual void HandleStopSprintRequest() override;

	/** handles a stop sprint request from player controller */
	virtual void HandleStartSprintRequest() override;

	virtual void HandleCrouchRequest() override;

	virtual void HandleItemEquipRequest(const int32 ItemId, const uint8 SlotIndex) override;

	virtual void HandleItemFinishUnEquipRequest() override;

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerEquipItem(const int32 NextItemId, const uint8 SlotIndex);

	virtual void EquipItem(const int32 NextItemId, const uint8 SlotIndex);

	virtual void SetCurrentAnimData(UINSStaticAnimData* AnimData) override;

	UFUNCTION()
	virtual void EquipGameModeDefaultWeapon();

	virtual void EquipBestWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerEquipBestWeapon();

	virtual void SetTeamType(const ETeamType NewTeamType);

	virtual void OnEnterIdleState() override;

	virtual void RecalculateBaseEyeHeight() override;

	virtual void OnOutIdleState() override;

	virtual void OnEnterBoredState() override;

	virtual void OnOutBoredState() override;

	virtual void OnLowHealth() override;

	virtual void TickRecoil(float DeltaSeconds);

	virtual void EquipFromInventory(const uint8 SlotIndex);

	virtual void UnEquipItem() override;

	virtual void ServerUnEquipItem() override;

	virtual void FinishUnEquipItem() override;

	virtual void Die() override;

	virtual AINSPlayerController* GetINSPlayerController() const { return INSPlayerController; }

	virtual void SetWeaponBasePoseType(const EWeaponBasePoseType NewType) override;

	virtual UINSFPAnimInstance* GetFPAnimInstance();

	virtual UINSTPAnimInstance* GetTPSAnimInstance();

	virtual void OnShotFired() override;

	virtual void OnReloadFinished() override;

	virtual void GetCacheCharAnimInstances(TArray<UINSCharacterAimInstance*>& OutAnimInstances);
	virtual bool CheckCharacterIsReady() override;
	virtual float PlayWeaponUnEquipAnim() override;
	virtual float PlayWeaponEquipAnim() override;
	virtual float PlayFireAnim() override;
	virtual float PlayWeaponReloadAnim() override;
	virtual float PlayWeaponSwitchFireModeAnim() override;
	virtual void ReceiveInventoryInitialized() override;
};
