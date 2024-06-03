// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "RPlayerState.generated.h"


class URInventoryComponent;


/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayerState : public APlayerState
{
	GENERATED_BODY()

	public:

	ARPlayerState();

	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	URInventoryComponent* PlayerInventory;
	


};
