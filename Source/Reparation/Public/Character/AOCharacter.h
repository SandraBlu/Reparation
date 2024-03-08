// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Attributes/AOAttributeSet.h"
#include "Interfaces/CombatInterface.h"
#include "AOCharacter.generated.h"



class UAOAbilitySystemComponent;
class UAOAttributeSet;
class UAOFootstepsComponent;
class AAOWeapon;

UCLASS()
class REPARATION_API AAOCharacter : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:

	AAOCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	
	void GrantAbilities();

protected:

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level) const;
	void InitializeAttributes()const ;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAttributeSet* AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	virtual void InitAbilityActorInfo();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> BaseAttributes;
	
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> PrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> SecondaryAttributes;

	///** List of GameplayEffects to apply when the Ability System Component is initialized (typically on begin play) */
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> GrantedAbilities;

	//Combat
	UPROPERTY(EditAnywhere, Category = "Weapon")
	AAOWeapon* EquippedWeapon;

	UFUNCTION(BlueprintPure)
	FORCEINLINE AAOWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	virtual FVector GetCombatSocketLocation() override;

	UPROPERTY(EditAnywhere, Category = "ProjectileSocket")
	FName HandSocketName;

	/**Combat Interface*/
	/*FORCEINLINE int32 GetLevel() override { return Level; }*/
	/**End Combat Interface*/

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XP")
	//int32 CharacterLevel = 1;



	//Footsteps Component
	UPROPERTY(BlueprintReadOnly)
	UAOFootstepsComponent* FootstepsComp;

	//Inventory Comp
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UAOInventoryComponent* LootSource;


};
