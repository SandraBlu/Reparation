// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "RPlayerState.generated.h"


class UAttributeSet;
class URInventoryComponent;

class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ARPlayerState();

	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	URInventoryComponent* PlayerInventory;

	UPROPERTY()
	UAttributeSet* AttributeSet;
    
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;


};
