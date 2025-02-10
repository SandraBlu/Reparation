// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "RAbilityTask_SpawnEnemies.generated.h"

class AREnemy;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitSpawnEnemiesDelegate, const TArray<AREnemy*>&, SpawnedEnemies);
/**
 * 
 */
UCLASS()
class REPARATION_API URAbilityTask_SpawnEnemies : public UAbilityTask
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Warrior|AbilityTasks", meta = (DisplayName = "Wait Event Spawn Enemies", HidePin = "OwningAbility",DefaultToSelf = "OwningAbility",BlueprintInternalUseOnly = "true",NumToSpawn = "1",RandomSpawnRadius = "200"))
	static URAbilityTask_SpawnEnemies* WaitSpawnEnemies(UGameplayAbility* OwningAbility,FGameplayTag EventTag,TSoftClassPtr<AREnemy> SoftEnemyClassToSpawn,int32 NumToSpawn,const FVector& SpawnOrigin,float RandomSpawnRadius);

	UPROPERTY(BlueprintAssignable)
	FWaitSpawnEnemiesDelegate OnSpawnFinished;

	UPROPERTY(BlueprintAssignable)
	FWaitSpawnEnemiesDelegate DidNotSpawn;

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	
	FGameplayTag CachedEventTag;
	TSoftClassPtr<AREnemy> CachedSoftEnemyClassToSpawn;
	int32 CachedNumToSpawn;
	FVector CachedSpawnOrigin;
	float CachedRandomSpawnRadius;
	FRotator CachedSpawnRotation;
	FDelegateHandle DelegateHandle;

	void OnGameplayEventReceived(const FGameplayEventData* InPayload);
	void OnEnemyClassLoaded();
	
};
