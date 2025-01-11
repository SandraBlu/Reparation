// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RLootable.generated.h"

UCLASS()
class REPARATION_API ARLootable : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ARLootable();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UStaticMeshComponent* LootContainerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class URInventoryComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UDataTable* LootTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	FIntPoint LootRolls;

protected:

	//UFUNCTION()
	//void OnInteract(class ARPlayer* Character);
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



};
