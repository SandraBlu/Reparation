// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RCharacter.h"
#include "Interface/RCombatInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
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

	//Combat Interface
	virtual int32 GetCharacterLevel_Implementation() override;
	virtual void Die() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	virtual void DirectionalHitReact(const FVector& ImpactPoint);
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	virtual FVector GetCombatSocketLocation_Implementation() override;
	//Combat Interface

	
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
	//virtual void InitializeAttributes() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Settings")
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
