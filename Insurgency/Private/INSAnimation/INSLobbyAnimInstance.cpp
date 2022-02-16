// Fill out your copyright notice in the Description page of Project Settings.


#include "INSAnimation/INSLobbyAnimInstance.h"
#include "INSCharacter/INSLobbyCharacter.h"
#include "INSItems/INSWeapons/INSWeaponBase.h"

UINSLobbyAnimInstance::UINSLobbyAnimInstance(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	CurrentActiveRelaxAnim = nullptr;
	WeaponBasePoseAnim = nullptr;
	OwnerLobbyCharacter = nullptr;
	LobbyWeapon = nullptr;
}

void UINSLobbyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerLobbyCharacter = Cast<AINSLobbyCharacter>(TryGetPawnOwner());
}

void UINSLobbyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	PlayWeaponBasePose();
	PlayRelaxAnim();
}

void UINSLobbyAnimInstance::PlayWeaponBasePose()
{
	if (WeaponBasePoseAnim && !Montage_IsPlaying(WeaponBasePoseAnim))
	{
		Montage_Play(WeaponBasePoseAnim);
	}
}

void UINSLobbyAnimInstance::PlayRelaxAnim()
{
	if (!CurrentActiveRelaxAnim && RelaxAnims.Num() > 1 && !Montage_IsPlaying(CurrentActiveRelaxAnim))
	{
		const int32 Random = FMath::RandHelper(RelaxAnims.Num() - 1);
		CurrentActiveRelaxAnim = RelaxAnims[Random];
		Montage_Play(CurrentActiveRelaxAnim);
	}
}

void UINSLobbyAnimInstance::UpDateWeaponBasePoseAnim(UAnimMontage* NewAnim)
{
	this->WeaponBasePoseAnim = NewAnim;
}

void UINSLobbyAnimInstance::SetLobbyWeapon(AINSWeaponBase* NewWeapon)
{
	this->LobbyWeapon = NewWeapon;
}
