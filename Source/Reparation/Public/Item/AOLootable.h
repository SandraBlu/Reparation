// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AOLootable.generated.h"

UCLASS()
class REPARATION_API AAOLootable : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AAOLootable();

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UStaticMeshComponent* LootContainerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UAOInteractComponent* LootInteract;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UAOInventoryComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UDataTable* LootTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	FIntPoint LootRolls;

protected:
	
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnInteract(class AAOPlayerBase* Character);

};
