// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AORestoreSite.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
class UNiagaraComponent;
class UAOInteractComponent;

UCLASS()
class REPARATION_API AAORestoreSite : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AAORestoreSite();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects")
	TSubclassOf<UGameplayEffect> EffectClass;

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

protected:

	UFUNCTION()
	void OnInteract(class AAOPlayerBase* Character);

	UPROPERTY()
	bool bIsActive;

	UFUNCTION()
	void IsActive();

	UPROPERTY(EditAnywhere, Category = "Powerup")
	float RespawnTime;

	FTimerHandle TimerHandle_RespawnTimer;

	UFUNCTION()
	void ShowRestoreSite();

	void HideAndCooldownRestoreSite();

	void SetRestoreSiteState(bool bNewIsActive);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UAOInteractComponent* InteractComp;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraComponent* RestoreSiteVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Effects")
	float ActorLevel = 1.f;

	
};
