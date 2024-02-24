// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AOCharacter.generated.h"

UCLASS()
class REPARATION_API AAOCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	AAOCharacter();

	//Inventory Comp
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UAOInventoryComponent* LootSource;

protected:
	
	virtual void BeginPlay() override;

};
