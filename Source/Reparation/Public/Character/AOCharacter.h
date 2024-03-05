// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Attributes/AOAttributeSet.h"
#include "AOCharacter.generated.h"


class UAOAbilitySystemComponent;
class UAOAttributeSet;
class UAOFootstepsComponent;

UCLASS()
class REPARATION_API AAOCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AAOCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	//virtual void InitializeAttributes();
	//virtual void GrantAbilities();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAttributeSet* AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	virtual void InitAbilityActorInfo();

	///** List of GameplayEffects to apply when the Ability System Component is initialized (typically on begin play) */
	//UPROPERTY(EditDefaultsOnly, Category = "GAS")
	//TSubclassOf<class UGameplayEffect> GrantedEffect;

	///** List of GameplayEffects to apply when the Ability System Component is initialized (typically on begin play) */
	//UPROPERTY(EditDefaultsOnly, Category = "GAS")
	//TArray<TSubclassOf<class UGameplayAbility>> GrantedAbilities;

	//Footsteps Component
	UPROPERTY(BlueprintReadOnly)
	UAOFootstepsComponent* FootstepsComp;

	//Inventory Comp
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UAOInventoryComponent* LootSource;


};
