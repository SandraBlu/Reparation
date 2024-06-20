// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RPlayerState.generated.h"

class URInventoryComponent;
class UAttributeSet;
class UAbilitySystemComponent;
class ULevelUpInfo;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32)
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

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	ULevelUpInfo* LevelUpInfo;
	
	FOnPlayerStatChanged OnXPChangeDelegate;
	FOnPlayerStatChanged OnLevelChangeDelegate;
	
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	FORCEINLINE int32 GetXP() const { return XP; }

	void AddToXP(int32 InXP);
	void AddToLevel(int32 InLevel);
	
	void SetXP(int32 InXP);
	void SetLevel(int32 InLevel);

protected:
	
	UPROPERTY(VisibleAnywhere)
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UAttributeSet* AttributeSet;

private:

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_XP)
	int32 XP = 1;
	
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);
	
	UFUNCTION()
	void OnRep_XP(int32 OldXP);
	
};
