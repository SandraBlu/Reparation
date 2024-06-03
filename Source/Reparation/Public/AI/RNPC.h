// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RCharacter.h"
#include "Interface/RCombatInterface.h"
#include "AbilitySystem/Data/RCharacterClassData.h"
#include "RNPC.generated.h"

class UWidgetComponent;

/**
 * 
 */
UCLASS()
class REPARATION_API ARNPC : public ARCharacter
{
	GENERATED_BODY()

public:

	ARNPC();
	
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 250.f;

	UFUNCTION(BlueprintImplementableEvent)
	void DissolveMesh();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	AActor* CombatTarget;

protected:
	
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitAbilityActorInfo() override;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Settings")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* Health;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USkeletalMeshComponent* Weapon;

	//Weapon Damage Socket
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName DamageSocket;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UPawnSensingComponent* PawnSensingComp;

	//UFUNCTION()
	//void OnPawnSeen(APawn* Pawn);

	//Montage
	void PlayHitReactMontage(const FName& SectionName);
	
private:

	//Montage
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* HitReact;

	UPROPERTY(EditAnywhere, Category = "SFX")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "VFX")
	class UNiagaraSystem* HitParticles;
};
