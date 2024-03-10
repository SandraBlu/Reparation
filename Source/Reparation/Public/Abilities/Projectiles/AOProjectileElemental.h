// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AOGameplayAbility.h"
#include "AOProjectileElemental.generated.h"

class AAOProjectile;

/**
 * 
 */
UCLASS()
class REPARATION_API UAOProjectileElemental : public UAOGameplayAbility
{
	GENERATED_BODY()

public:

	UAOProjectileElemental();

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/* Sphere radius of the sweep to find desired target under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepRadius;

	/* Fallback distance when sweep finds no collision under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepDistanceFallback;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAOProjectile> ProjectileClass;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(AAOPlayerBase* InstigatorCharacter);
};
