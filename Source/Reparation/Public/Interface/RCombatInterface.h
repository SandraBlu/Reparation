// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Data/RNPCData.h"
#include "RCombatInterface.generated.h"

class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);


USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CombatSocketTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REPARATION_API IRCombatInterface
{
	GENERATED_BODY()

	
public:
	
	UFUNCTION(BlueprintNativeEvent)
	void GetHit(const FVector& ImpactPoint);

	UFUNCTION(BlueprintNativeEvent)
	int32 GetCharacterLevel();


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVector GetCombatSocketLocation(const FGameplayTag& CombatSocketTag);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void FaceTarget(const FVector& Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	virtual void Die() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetCombatTarget(AActor* InCombatTarget);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetCombatTarget() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetAvatar();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FTaggedMontage> GetAttackMontages();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UNiagaraSystem* GetBloodEffect();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FTaggedMontage GetTaggedMontageByTag(const FGameplayTag& MontageTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	ENPCClass GetNPCClass();

	virtual FOnDeath GetOnDeathDelegate() = 0;
};
