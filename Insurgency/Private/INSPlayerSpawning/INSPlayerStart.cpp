// Fill out your copyright notice in the Description page of Project Settings.


#include "INSPlayerSpawning/INSPlayerStart.h"

AINSPlayerStart::AINSPlayerStart(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PlayerStartTag = TEXT("CT");
//#if WITH_EDITOR&&!UE_BUILD_SHIPPING
//	bShowPlayerStartOverlapDebugSphere = true;
//#endif
}

//#if WITH_EDITOR&&!UE_BUILD_SHIPPING
//void AINSPlayerStart::DrawPlayerStartPointOverlapSphere()
//{
//
//}
//#endif
