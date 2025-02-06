// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RDamageAbility.h"
#include "RProjectileAbility.generated.h"

class UGameplayEffect;
class AROverlapMissile;
/**
 * 
 */
UCLASS()
class REPARATION_API URProjectileAbility : public URDamageAbility
{
	GENERATED_BODY()

public:

	URProjectileAbility();

protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	/* Sphere radius of the sweep to find desired target under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepRadius;

	/* Fallback distance when sweep finds no collision under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepDistanceFallback;
	
	UPROPERTY(EditDefaultsOnly)
	int32 NumProjectiles = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AROverlapMissile> ProjectileClass;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(ACharacter* InstigatorCharacter, const FGameplayTag& SocketTag);
};
