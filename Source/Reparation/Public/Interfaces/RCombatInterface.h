// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "RCombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REPARATION_API IRCombatInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent)
	void GetHit(const FVector& ImpactPoint);

	UFUNCTION(BlueprintNativeEvent)
	int32 GetPLayerLevel();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation(const FGameplayTag& CombatSocketTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();
	
	virtual void Die() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsDead() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void FaceTarget(const FVector& Target);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetAvatar();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetCombatTarget(AActor* InCombatTarget);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetCombatTarget() const;

};
