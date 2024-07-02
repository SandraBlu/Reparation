// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RPlayerController.generated.h"

class ATargetCircle;
class UDamageTextComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ARPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bDodgedHit, bool bCriticalHit);
	
	UFUNCTION(BlueprintCallable)
	void ShowTargetingCircle(UMaterialInterface* DecalMaterial = nullptr);
	UFUNCTION(BlueprintCallable)
	void HideTargetingCircle();

private:

	void CursorTrace();
	FHitResult CursorHit;
	
	UPROPERTY(EditDefaultsOnly, Category = Target)
	TSubclassOf<ATargetCircle> TargetCircleClass;
        
	UPROPERTY()
	ATargetCircle* TargetCircle;

	void UpdateMagicCircleLocation();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
	
};
