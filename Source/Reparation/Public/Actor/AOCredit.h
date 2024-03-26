// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AOCredit.generated.h"


UCLASS()
class REPARATION_API AAOCredit : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AAOCredit();

	UPROPERTY(VisibleAnywhere)
	class AAOPlayerController* PC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UStaticMeshComponent* CreditMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UAOInteractComponent* CreditInteract;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category  = "Components")
	class UAOInventoryComponent* Inventory;

protected:

	UPROPERTY(EditAnywhere, Category = "Credit")
	int32 CreditAmount;
};
