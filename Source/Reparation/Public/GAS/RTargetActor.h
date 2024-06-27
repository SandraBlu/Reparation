// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "RTargetActor.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API ARTargetActor : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()


public:
	
	ARTargetActor();

	AGameplayAbilityWorldReticle* ReticleActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "GAS")
	float TraceRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FHitResult TraceHitResults;

	bool LineTraceFN(FHitResult& TraceHitResult) const;

	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;
	virtual void CancelTargeting() override;

	virtual AGameplayAbilityWorldReticle* SpawnReticleActor(FVector Location, FRotator Rotation);
	virtual void DestroyReticleActors();
	
};
