// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/RGameplayAbility.h"
#include "RMagicMissile.generated.h"
class ARProjectile;
/**
 * 
 */
UCLASS()
class REPARATION_API URMagicMissile : public URGameplayAbility
{
	GENERATED_BODY()

public:

	URMagicMissile();
	

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	/* Sphere radius of the sweep to find desired target under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepRadius;

	/* Fallback distance when sweep finds no collision under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepDistanceFallback;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ARProjectile> ProjectileClass;
	
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(ACharacter* InstigatorCharacter);

	UFUNCTION(BlueprintImplementableEvent)
	void GetHit(const FVector& Location);
};
