// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ReparationEditor : ModuleRules
{
	public ReparationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Reparation" });
	}
}
