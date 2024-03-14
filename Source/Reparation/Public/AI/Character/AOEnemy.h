// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "UI/WidgetController//OverlayWidgetController.h"
#include "Abilities/Data/CharacterClassInfo.h"
#include "AOEnemy.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AAOAIController;

/**
 * 
 */
UCLASS()
class REPARATION_API AAOEnemy : public AAOCharacter
{
	GENERATED_BODY()

public:

	AAOEnemy();
	virtual void PossessedBy(AController* NewController) override;

	//Combat Interface
	virtual int32 GetPlayerLevel() override;

	virtual void Die() override;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChange OnHealthChange;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChange OnMaxHealthChange;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolve();

protected:

	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeAttributes() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Settings")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Settings")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* Health;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USkeletalMeshComponent* Weapon;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BTree;

	UPROPERTY()
	AAOAIController* AIC;


};
