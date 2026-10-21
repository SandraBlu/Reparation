#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "EnemyBlueprintEditorLibrary.generated.h"

class UBlueprint;
class UAnimMontage;
class UBehaviorTree;

/** Editor helpers for reviewing and repairing existing enemy graphs. */
UCLASS()
class REPARATIONEDITOR_API UEnemyBlueprintEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** Remove only GA_Melee's legacy global-dilation/delay/restore chain; does not save. */
	UFUNCTION(BlueprintCallable, Category="Combat|Editor")
	static bool RemovePlayerMeleeHitSlowdown(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Skills|Editor")
	static bool WireKatanaComboCompletion(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static FString DescribeFunctionGraph(UBlueprint* Blueprint, FName GraphName);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static bool GuardFacingTarget(UBlueprint* Blueprint);
	/** Remove hit-processing exits that stop the attack before montage completion. */
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static int32 KeepMeleeMontagePlaying(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static FString DescribeMeleeSetup(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static bool WireMeleeHeadQuery(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static bool AddMaceHeadWindow(UAnimMontage* Montage, FGameplayTag EventTag, float StartTime, float EndTime);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static FString DescribeBehaviorTree(UBehaviorTree* Tree);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static bool EnsureMeleePosition(UBehaviorTree* Tree, FName AttackTaskClassName);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static bool ConfigureMaceStances(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static bool FixMeleeBlocking(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Combat|Editor") static bool WireSuccessfulBlockNotification(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="Enemy|Editor")
	static bool ConfigurePerceptionSearch(UBehaviorTree* Tree);
};
