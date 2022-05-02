// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class InsurgencyTarget : TargetRules
{
	public InsurgencyTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new string[] { "Insurgency" });
		//bCompileChaos = true;
		//bUseChaos = true;
		//bCompilePhysX = true;
		//bCompileAPEX = true;
		//bCompileNvCloth = true;
		//bCustomSceneQueryStructure = true;
		//BuildEnvironment = TargetBuildEnvironment.Unique;
	}
}