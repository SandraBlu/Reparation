// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "REnemyCombatComponent.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UREnemyCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:
	
	virtual void OnHitTargetActor(AActor* HitActor);

	UPROPERTY()
	TArray<AActor*> OverlappedActors;
	
};
