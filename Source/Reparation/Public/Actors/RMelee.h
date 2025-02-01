// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/RWeapon.h"
#include "RMelee.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API ARMelee : public ARWeapon
{
	GENERATED_BODY()

public:
	
	ARMelee();
	
	//UFUNCTION(BlueprintCallable)
	//virtual void OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	//UFUNCTION(BlueprintImplementableEvent)
	//void OnHit(FHitResult BoxHit);
	
	//Actors to ignore after weapon hits them (same swing)
	//UPROPERTY()
	//TArray<AActor*> IgnoreActors;

protected:

	//Implement Field System on Weapon
	//UFUNCTION(BlueprintImplementableEvent)
	//void CreateForceFields(const FVector& FieldLocation);

private:
	
	virtual void BeginPlay() override;
	
	
	
};
