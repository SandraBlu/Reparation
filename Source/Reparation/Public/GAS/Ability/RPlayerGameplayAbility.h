// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RDamageAbility.h"
#include "RPlayerGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URPlayerGameplayAbility : public URDamageAbility
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	ARPlayerController* GetPlayerControllerFromActorInfo();
	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	class UPlayerCombatComp* GetPlayerCombatComponentFromActorInfo();
	
private:
	
	TWeakObjectPtr<ARPlayerController> CachedWarriorHeroController;
	
};
