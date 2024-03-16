// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileHit, const FHitResult&, DataHandle);

UCLASS()
class REPARATION_API UTargetData : public UAbilityTask
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetData* GetProjectileTargetData(UGameplayAbility* OwningAbility);

	UPROPERTY(BlueprintAssignable)
	FOnProjectileHit HitData;

private:

	virtual void Activate() override;
};
