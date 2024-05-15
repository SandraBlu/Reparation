// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "RCharacter.generated.h"

class UAbilitySystemComponent;

UCLASS()
class REPARATION_API ARCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARCharacter();

	// Overridden from Ability System Interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

	virtual void InitAbilityActorInfo();
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level) const;
	virtual void InitializeAttributes() const;
	void GrantAbilities();

	//UPROPERTY()
	//UAttributeSet* AttributeSet;

	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> BaseAttributes;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> PrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> SecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> ResistanceAttributes;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> GrantedAbilities;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> PassiveAbilities;

	//Combat Interface
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XP")
	int32 CharacterLevel = 1;

	//Combat
	///UPROPERTY(EditAnywhere, Category = "Combat")
	//AAOWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RHand;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LHand;

	bool bDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	class UNiagaraSystem* BloodEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USoundBase* DeathSFX;

	//class UAOAbilityComp* AOASC;

	//class UAOAbilityComp* GetASC();

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

};
