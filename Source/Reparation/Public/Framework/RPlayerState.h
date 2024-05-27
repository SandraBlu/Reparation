// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "RPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class URInventoryComponent;
//class UXPInfo;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/)
/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

	public:

	ARPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() { return AttributeSet; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	URInventoryComponent* PlayerInventory;

	FOnPlayerStatChanged OnXPChangeDelegate;
	FOnPlayerStatChanged OnLevelChangeDelegate;
	FOnPlayerStatChanged OnAttributePtsChangeDelegate;
	FOnPlayerStatChanged OnAbilityPtsChangeDelegate;

	FORCEINLINE int32 GetCharacterLevel() const { return Level; }
	FORCEINLINE int32 GetXP() const { return XP; }
	FORCEINLINE int32 GetAttributePts() const { return AttributePts; }
	FORCEINLINE int32 GetAbilityPts() const { return AbilityPts; }

	void AddToLevel(int32 InLevel);
	void AddToXP(int32 InXP);
	void AddToAttributePts(int32 InAttributePts);
	void AddToAbilityPts(int32 InAbilityPts);

	void SetLevel(int32 InLevel);
	void SetXP(int32 InXP);


protected:

	UPROPERTY()
	UAttributeSet* AttributeSet;

	UPROPERTY(VisibleAnywhere)
	UAbilitySystemComponent* AbilitySystemComponent;

	
private:

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;

	UFUNCTION()
	void OnRep_Level(int32 OldLevel);
	
	UPROPERTY(VisibleAnywhere)
	int32 XP = 1;

	UPROPERTY(VisibleAnywhere)
	int32 AttributePts = 0;

	UPROPERTY(VisibleAnywhere)
	int32 AbilityPts = 0;
};
