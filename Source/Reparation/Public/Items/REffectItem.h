// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/RItem.h"
#include "REffectItem.generated.h"


/**
 * 
 */
UCLASS()
class REPARATION_API UREffectItem : public URItem
{
	GENERATED_BODY()

public:	

	UREffectItem();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EffectActor")
	TSubclassOf<class AREffectActor> EffectClass;

};