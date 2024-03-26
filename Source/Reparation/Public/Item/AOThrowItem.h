// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/AOEquipItem.h"
#include "AOThrowItem.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UAOThrowItem : public UAOEquipItem
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	class UAnimMontage* TossAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons")
	TSubclassOf<class AAOThrowable> ThrowableClass;
	
};
