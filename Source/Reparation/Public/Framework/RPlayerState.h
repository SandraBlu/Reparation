// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RPlayerState.generated.h"

class URInventoryComponent;
class UAttributeSet;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	ARPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	URInventoryComponent* PlayerInventory;

	FORCEINLINE int32 GetPlayerLevel() const { return Level; }

protected:
	
	UPROPERTY(VisibleAnywhere)
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UAttributeSet* AttributeSet;

private:

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;
	
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);
	
};
