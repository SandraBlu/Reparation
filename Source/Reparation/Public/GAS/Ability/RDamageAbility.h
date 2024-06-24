// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RGameplayAbility.h"
#include "Interfaces/RCombatInterface.h"
#include "RDamageAbility.generated.h"
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class REPARATION_API URDamageAbility : public URGameplayAbility
{
	GENERATED_BODY()

public:

	UFUNCTION(Blueprintcallable)
	void CauseDamage(AActor* TargetActor);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;
	
	UFUNCTION(BlueprintPure)
	FTaggedMontage GetRandomTaggedMontage(const TArray<FTaggedMontage>& TaggedMontages) const;

	float GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType);
	
};
