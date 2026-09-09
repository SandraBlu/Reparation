// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RDamageAbility.h"
#include "RFireBlast.generated.h"

class ARFireball;
/**
 * 
 */
UCLASS()
class REPARATION_API URFireBlast : public URDamageAbility
{
	GENERATED_BODY()

public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
	
	UFUNCTION(BlueprintCallable)
	TArray<ARFireball*> SpawnFireballs();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "FireBlast")
	int32 NumFireballs = 12;

private:

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ARFireball> FireballClass;
	
};
