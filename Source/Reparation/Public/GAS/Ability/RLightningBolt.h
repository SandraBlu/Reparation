// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RMissileAbility.h"
#include "RLightningBolt.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URLightningBolt : public URMissileAbility
{
	GENERATED_BODY()

public:
	
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectiles(ACharacter* InstigatorCharacter, bool bOverridePitch, float PitchOverride, AActor* HomingTarget);

	UPROPERTY(EditDefaultsOnly)
	int32 MaxNumProjectiles = 5;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float HomingAccelerationMin = 1600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float HomingAccelerationMax = 3200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	bool bLaunchHomingProjectiles = true;
};
