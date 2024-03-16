// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AOProjectile.generated.h"


class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UAudioComponent;
class UCameraShakeBase;
class UNiagaraSystem;
class USoundBase;


UCLASS(ABSTRACT)
class REPARATION_API AAOProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AAOProjectile();

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* MovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	UNiagaraComponent* EffectComp;

		// 'virtual' so we can override this in child-classes
	UFUNCTION()
	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// BlueprintNativeEvent = C++ base implementation, can be expanded in Blueprints
	// BlueprintCallable to allow child classes to trigger explosions
	// Not required for assignment, useful for expanding in Blueprint later on
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Explode();

	virtual void PostInitializeComponents() override;



private:

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* ImpactVFX;

	UPROPERTY(EditAnywhere, Category = "Components")
	USoundBase* ImpactSFX;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UAudioComponent* LaunchSFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float Lifespan = 10.f;
	
};