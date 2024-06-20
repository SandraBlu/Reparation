// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/RCharacterBase.h"
#include "UI/GAS/Controllers/ROverlayWidgetController.h"
#include "RNPC.generated.h"

class UWidgetComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API ARNPC : public ARCharacterBase
{
	GENERATED_BODY()

public:
	
	ARNPC();

	//Combat Interface
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag) override;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMaxHealthChanged;
	
protected:
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* HealthBar;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	USkeletalMeshComponent* Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponDamageSocket;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;
	
};
