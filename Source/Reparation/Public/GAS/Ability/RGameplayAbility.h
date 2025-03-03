// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "RGameplayAbility.generated.h"

class URAbilitySystemComponent;
class UPlayerCombatComp;
class ARPlayerController;
class ARPlayer;
/**
 * 
 */
UCLASS()
class REPARATION_API URGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;

	virtual FString GetDescription(int32 Level);
	virtual FString GetNextLevelDescription(int32 Level);
	static FString GetLockedDescription(int32 Level);

protected:

	float GetEnergyCost(float InLevel = 1.f) const;
	float GetCooldown(float InLevel = 1.f) const;

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	URAbilitySystemComponent* GetRAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	ARPlayer* GetPlayerFromActorInfo();
	
	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	virtual class AREnemy* GetEnemyCharacterFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	virtual class ARPlayerController* GetPlayerControllerFromActorInfo();

private:
	
	TWeakObjectPtr<ARPlayer> CachedPlayer;
	TWeakObjectPtr<AREnemy> CachedEnemy;
	TWeakObjectPtr<ARPlayerController> CachedPlayerController;
};


