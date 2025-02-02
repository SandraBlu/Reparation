// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RPlayerGameplayAbility.h"
#include "TargetLockAbility.generated.h"

class UCombatWidget;
/**
 * 
 */
UCLASS()
class REPARATION_API UTargetLockAbility : public URPlayerGameplayAbility
{
	GENERATED_BODY()

protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable)
	void OnTargetLockTick(float DeltaTime);
	
private:
	void TryLockOnTarget();
	void GetAvailableActorsToLockOnto();
	AActor* GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors);
	void DrawTargetLockWidget();
	void SetTargetLockWidgetPosition();
	
	void CancelTargetLockAbility();
	void CleanUp();
	
	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	float BoxTraceDistance = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	FVector TraceBoxSize = FVector(5000.f,5000.f,300.f);

	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	TArray< TEnumAsByte < EObjectTypeQuery > > BoxTraceChannel;

	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	bool bShowPersistentDebugShape = false;

	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	float TargetLockRotationInterpSpeed = 5.f;
	
	UPROPERTY()
	TArray<AActor*> AvailableTargetLockActors;
	UPROPERTY()
	AActor* CurrentLockedActor;
	
	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	TSubclassOf<UCombatWidget> TargetLockWidgetClass;

	UPROPERTY()
	UCombatWidget* DrawnTargetLockWidget;
	
	UPROPERTY()
	FVector2D TargetLockWidgetSize = FVector2D::ZeroVector;
	
};
