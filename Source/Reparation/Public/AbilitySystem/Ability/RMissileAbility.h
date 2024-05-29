// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/RGameplayAbility.h"
#include "RMissileAbility.generated.h"

class ARProjectile;
/**
 * 
 */
UCLASS()
class REPARATION_API URMissileAbility : public URGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	URMissileAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ARProjectile> ProjectileClass;


	
};
