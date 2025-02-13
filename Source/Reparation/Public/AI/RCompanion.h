// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/RCharacterBase.h"
#include "RCompanion.generated.h"

class UBehaviorTree;
class AAIC_Companion;
/**
 * 
 */
UCLASS()
class REPARATION_API ARCompanion : public ARCharacterBase
{
	GENERATED_BODY()

public:
	
	ARCompanion();
	
	virtual void PossessedBy(AController* NewController) override;


protected:
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	UBehaviorTree* BT_Companion;

	UPROPERTY()
	AAIC_Companion* AIC_Companion;
	
};
