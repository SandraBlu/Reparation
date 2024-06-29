// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RMissileAbility.h"
#include "RFlamethrower.generated.h"

class AROverlapMissile;
/**
 * 
 */
UCLASS()
class REPARATION_API URFlamethrower : public URMissileAbility
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
	void GetOwnerInfo();
	
	UFUNCTION(BlueprintCallable)
	void FindTarget(FVector& TargetLocation);

protected:

	UPROPERTY(BlueprintReadWrite, Category = "GAS")
	ACharacter* OwnerCharacter;
	
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	AActor* CurrentTarget;

	
	
};
