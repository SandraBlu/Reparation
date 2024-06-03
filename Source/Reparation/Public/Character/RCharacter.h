// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "RCharacter.generated.h"


class UAbilitySystemComponent;
class URFootstepsComponent;
class UGameplayAbility;
class UREquipmentComponent;

UCLASS()
class REPARATION_API ARCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ARCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UREquipmentComponent* Gear;
	
protected:

	UPROPERTY()
	UAttributeSet* AttributeSet;

	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RHand;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LHand;
	
	//Footsteps Comp getter-----------------
	URFootstepsComponent* GetFootstepsComp() const;
	
	UPROPERTY(BlueprintReadOnly)
	URFootstepsComponent* FootstepsComp;

	bool bDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	class UNiagaraSystem* BloodEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USoundBase* DeathSFX;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

private:

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

};
