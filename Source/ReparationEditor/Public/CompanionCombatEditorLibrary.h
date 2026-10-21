#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "CompanionCombatEditorLibrary.generated.h"

class UAnimBlueprint;
class UBlueprint;
class USkeletalMesh;
class ARCompanion;
class UCurveTable;

/** Editor setup used by the companion combat asset script. */
UCLASS()
class REPARATIONEDITOR_API UCompanionCombatEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static bool ConsolidateSkillTreeMenu();
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static bool StyleTreeLoadoutIcons();
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static bool SetupTreeLoadout();
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static bool AddSkillRespecControl();
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static bool CleanLegacyLoadout();
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static bool CreateSkillTreeDesignerAssets();
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static bool CreateSkillBrowserDesignerAssets();
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static FString DescribeWidgetRoot(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Skills|Editor") static FString DescribeWidgetLayout(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Combat|Editor")
	static FString DescribeDamageCurves(UCurveTable* Table);
	UFUNCTION(BlueprintCallable, Category="Companion|Editor")
	static FString GetActorPackageFile(ARCompanion* Companion);
	UFUNCTION(BlueprintCallable, Category="Companion|Editor")
	static bool RestoreEmptyWeaponSlots(ARCompanion* Companion);
	UFUNCTION(BlueprintCallable, Category="Companion|Editor")
	static bool ConfigureWeaponSlots(UBlueprint* Blueprint, USkeletalMesh* RightMesh, USkeletalMesh* LeftMesh, bool bRequireWeapon);
	/** Insert a montage slot between the existing final pose and output, preserving locomotion. */
	UFUNCTION(BlueprintCallable, Category="Companion|Editor")
	static bool EnsureMontageSlot(UAnimBlueprint* Blueprint, FName SlotName);
	UFUNCTION(BlueprintPure, Category="Companion|Editor")
	static FGameplayTag GetRegisteredTag(FName TagName);
};
