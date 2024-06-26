// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RAbilityTypes.h"
#include "Actors/Projectiles/RProjectile.h"
#include "ROverlapMissile.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API AROverlapMissile : public ARProjectile
{
	GENERATED_BODY()

public:
	
	AROverlapMissile();
	
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FDamageEffectParams DamageEffectParams;

protected:
	
	UFUNCTION()
	void OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	virtual void PostInitializeComponents() override;
	void OnHit();

	bool bHit = false;
	virtual void Destroyed() override;
	
};
