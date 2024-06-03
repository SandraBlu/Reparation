// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Interface/RCombatInterface.h"
#include "GameplayTagContainer.h"
#include "RCharacter.generated.h"


class UAbilitySystemComponent;
class URFootstepsComponent;
class UGameplayAbility;
class UREquipmentComponent;

UCLASS()
class REPARATION_API ARCharacter : public ACharacter, public IAbilitySystemInterface, public IRCombatInterface
{
	GENERATED_BODY()

public:

	ARCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UREquipmentComponent* Gear;
	
protected:

	virtual void InitAbilityActorInfo();
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level) const;
	virtual void InitializeAttributes() const;
	void GrantAbilities();

	UPROPERTY()
	UAttributeSet* AttributeSet;

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
	
	//Combat Interface
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag) override;
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual AActor* GetAvatar_Implementation() override;
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation();
	virtual UNiagaraSystem* GetBloodEffect_Implementation();
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag);
	virtual void Die() override;
	virtual bool IsDead_Implementation() const override;
	virtual FOnDeath GetOnDeathDelegate() override;
	////Combat Interface

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "XP")
	int32 CharacterLevel = 1;

	FOnDeath  OnDeath;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedMontage> AttackMontages;

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
	
	UPROPERTY()
	class URAbilitySystemComponent* RASC;

	class URAbilitySystemComponent* GetRASC();

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

private:

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

};
