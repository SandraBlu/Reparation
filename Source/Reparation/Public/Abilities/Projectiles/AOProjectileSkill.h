// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AODamageAbility.h"
#include "AOProjectileSkill.generated.h"

class AAOMagicMissile;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class REPARATION_API UAOProjectileSkill : public UAODamageAbility
{
	GENERATED_BODY()

public:

	UAOProjectileSkill();

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	
	/* Sphere radius of the sweep to find desired target under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepRadius;

	/* Fallback distance when sweep finds no collision under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepDistanceFallback;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(ACharacter* InstigatorCharacter);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAOMagicMissile> ProjectileClass;

	UFUNCTION(BlueprintImplementableEvent)
	void GetHit(const FVector& Location);

};
