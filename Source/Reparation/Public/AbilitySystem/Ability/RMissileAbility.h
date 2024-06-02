// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/RGameplayAbility.h"
#include "RMissileAbility.generated.h"

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
	TSubclassOf<class ARProjectile> ProjectileClass;

	/* Sphere radius of the sweep to find desired target under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepRadius;

	/* Fallback distance when sweep finds no collision under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepDistanceFallback;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(ACharacter* InstigatorCharacter);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UGameplayEffect> DamageEffectClass;
	
};
