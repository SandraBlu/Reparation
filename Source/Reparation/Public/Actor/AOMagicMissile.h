// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/AOProjectile.h"
#include "GameplayEffectTypes.h"
#include "AOMagicMissile.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API AAOMagicMissile : public AAOProjectile
{
	GENERATED_BODY()

public:
	AAOMagicMissile();

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

protected:

	UFUNCTION()
	void OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void PostInitializeComponents() override;
};
