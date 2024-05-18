// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "RGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;
	
};
