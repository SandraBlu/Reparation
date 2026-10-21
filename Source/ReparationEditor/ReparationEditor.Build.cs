// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ReparationEditor : ModuleRules
{
	public ReparationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd", "GameplayTags" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Reparation", "AnimGraph", "BlueprintGraph", "AnimGraphRuntime", "GameplayAbilities", "AIModule", "BehaviorTreeEditor", "AIGraph", "UMG", "UMGEditor", "SlateCore" });
	}
}
