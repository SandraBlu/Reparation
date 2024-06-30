// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "PassiveNiagaraComponent.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UPassiveNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:

	UPassiveNiagaraComponent();

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PassiveSpellTag;
	
protected:

	virtual void BeginPlay();
	void OnPassiveActive(const FGameplayTag& AbilityTag, bool bActivate);
};
