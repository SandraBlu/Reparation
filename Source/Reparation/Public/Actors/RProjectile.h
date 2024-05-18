// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class USoundBase;
class UNiagaraSystem;
UCLASS()
class REPARATION_API ARProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	ARProjectile();

protected:
	
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* MovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	UNiagaraComponent* EffectComp;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* ImpactVFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	USoundBase* ImpactSound;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float Lifespan = 5;

	virtual void PostInitializeComponents() override;

};
