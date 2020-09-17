// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "INSWeaponMeshComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponSocketNames {

	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFWeaponMesh|SocketNames")
		FName MuzzleFlashSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFWeaponMesh|SocketNames")
		FName ShellEjectSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFWeaponMesh|SocketNames")
		FName ScopeHolderSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFWeaponMesh|SocketNames")
		FName LeftSideAttachSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFWeaponMesh|SocketNames")
		FName RightSideAttachSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BFWeaponMesh|SocketNames")
		FName GripAttachSocket;

	/** set default values */
	FWeaponSocketNames()
	{
		MuzzleFlashSocket = FName(TEXT("BarrelStart"));
		ScopeHolderSocket = FName(TEXT("ScopeHolderSocket"));
		LeftSideAttachSocket = FName(TEXT("LeftSideAttachSocket"));
		RightSideAttachSocket = FName(TEXT("RightSideAttachSocket"));
		GripAttachSocket = FName(TEXT("GripAttachSocket"));
		ShellEjectSocket = FName(TEXT("ShellEjection"));
	}
};


/**
 * 
 */
UCLASS()
class INSURGENCY_API UINSWeaponMeshComponent : public USkeletalMeshComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSkeletalMesh|AttachmentSlot")
		FWeaponSocketNames WeaponSockets;

// 	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSkeletalMesh|AttachmentSlot")
// 		FWeaponAttachmentSlot ScopeSlot;
// 
// 	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSkeletalMesh|AttachmentSlot")
// 		FWeaponAttachmentSlot GripSlot;
// 
// 	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSkeletalMesh|AttachmentSlot")
// 		FWeaponAttachmentSlot SilencerSlot;
// 
// 	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSkeletalMesh|AttachmentSlot")
// 		FWeaponAttachmentSlot LeftSlot;
// 
// 	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSkeletalMesh|AttachmentSlot")
// 		FWeaponAttachmentSlot RightSlot;
// 
// 	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSkeletalMesh|AttachmentSlot")
// 		FWeaponAttachmentSlot UnderSlot;
// 
// 	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponSkeletalMesh|AttachmentSlot")
// 		FWeaponAttachmentSlot IronSightSlot;
protected:
	virtual void BeginPlay()override;

public:
	virtual FWeaponSocketNames GetWeaponSockets()const { return WeaponSockets; }
	virtual FVector GetMuzzleLocation()const { return GetSocketLocation(WeaponSockets.MuzzleFlashSocket); }
	virtual FRotator GetMuzzleRotation()const { return GetSocketRotation(WeaponSockets.MuzzleFlashSocket); }
	virtual FVector GetMuzzleForwardVector()const { return GetMuzzleRotation().Vector(); }
	virtual FTransform GetMuzzleTransform()const { return GetSocketTransform(WeaponSockets.MuzzleFlashSocket); }
	virtual FVector GetShellSocketLocation()const { return GetSocketLocation(WeaponSockets.ShellEjectSocket); }
	virtual FTransform GetShellSpawnTransform()const;
};
