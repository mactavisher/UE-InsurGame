// Fill out your copyright notice in the Description page of Project Settings.


#include "INSCharacter/INSLobbyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "INSComponents/INSWeaponMeshComponent.h"
#include "INSAssets/INSStaticAnimData.h"
#include "Camera/CameraComponent.h"
#include "INSAnimation/INSLobbyAnimInstance.h"

// Sets default values
AINSLobbyCharacter::AINSLobbyCharacter(const FObjectInitializer&ObjectInitializer) :Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    //GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	//GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->AddRelativeLocation(FVector(0.f,0.f,-94.f));
	LobbyCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	LobbyCamera->bUsePawnControlRotation =false;
}

// Called when the game starts or when spawned
void AINSLobbyCharacter::BeginPlay()
{
	Super::BeginPlay();
	if(LobbyCamera)
	{
		LobbyCamera->AttachToComponent(GetCapsuleComponent(),FAttachmentTransformRules::SnapToTargetIncludingScale,NAME_None);
		LobbyCamera->AddRelativeLocation(FVector(200.f,50.f,65.f));
		LobbyCamera->AddRelativeRotation(FRotator(0.f,-180.f,0.f));
	}
	CreateClientLobbyWeapon();
}

void AINSLobbyCharacter::CreateClientLobbyWeapon()
{
	if(LobbyWeaponClass)
	{
		LobbyWeapon = GetWorld()->SpawnActorDeferred<AINSWeaponBase>(LobbyWeaponClass,GetActorTransform(),this,this,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		LobbyWeapon->GetWeaponMeshComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(LobbyWeapon)
		{
			UGameplayStatics::FinishSpawningActor(LobbyWeapon,GetActorTransform());
		}
		LobbyWeapon->GetWeaponMeshComp()->SetCollisionResponseToAllChannels(ECR_Ignore);
		LobbyWeapon->GetWeaponMeshComp()->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Bip01_Weapon1Socket"));
		UpdateAnimation();
	}
}

void AINSLobbyCharacter::UpdateAnimation()
{
	UINSLobbyAnimInstance* LobbyAnimInstance = Cast<UINSLobbyAnimInstance>(GetMesh()->AnimScriptInstance);
	if(LobbyAnimInstance)
	{
		LobbyAnimInstance->SetLobbyWeapon(LobbyWeapon);
		EWeaponBasePoseType CurrentWeaponPoseType = LobbyWeapon->GetCurrentWeaponBasePose();
		UAnimMontage* CurrentBasePoseAnim = CurrentWeaponPoseType==EWeaponBasePoseType::ALTGRIP?LobbyWeapon->GetWeaponAnimDataPtr()->FPWeaponAltGripAnim.BasePoseAnim.CharAnim:
		LobbyWeapon->GetWeaponAnimDataPtr()->FPWeaponForeGripAnim.BasePoseAnim.CharAnim;
		LobbyAnimInstance->UpDateWeaponBasePoseAnim(CurrentBasePoseAnim);
	}
}

UAnimMontage* AINSLobbyCharacter::GetWeaponBasePose() const
{
	if(LobbyWeapon)
	{
		if(LobbyWeapon->GetWeaponAnimDataPtr())
		{
			return LobbyWeapon->GetWeaponAnimDataPtr()->TPWeaponForeGripAnim.BasePoseAnim.CharAnim;
		}
	}
	return nullptr;
}

// Called every frame
void AINSLobbyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AINSLobbyCharacter::SetCurrentLobbyWeapon(int32 WeaponId)
{
}


