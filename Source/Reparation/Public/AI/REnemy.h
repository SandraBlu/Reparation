// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/RCharacterBase.h"
#include "UI/GAS/Controllers/ROverlayWidgetController.h"
#include "REnemy.generated.h"

class UTextBlock;
class UPawnCombatComponent;
class UPawnSensingComponent;
class ARAIController;
class UBehaviorTree;
class UWidgetComponent;
class UREnemyMeleeSweepComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API AREnemy : public ARCharacterBase
{
	GENERATED_BODY()

public:

	AREnemy(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;
	USkeletalMeshComponent* GetMeleeDamageMesh() const;
	FName GetMeleeDamageSocket() const { return WeaponDamageSocket; }
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UREnemyMeleeSweepComponent> MeleeSweep;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Melee Spacing|Crouched", meta=(ClampMin="1", DisplayName="Crouched Minimum Distance"))
	float MinMeleeDistance = 150.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Melee Spacing|Crouched", meta=(ClampMin="1", DisplayName="Crouched Preferred Distance"))
	float PreferredMeleeDistance = 190.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Melee Spacing|Crouched", meta=(ClampMin="1", DisplayName="Crouched Maximum Distance"))
	float MaxMeleeDistance = 230.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Melee Spacing|Standing", meta=(ClampMin="1"))
	float StandingMinMeleeDistance = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Melee Spacing|Standing", meta=(ClampMin="1"))
	float StandingPreferredMeleeDistance = 120.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Melee Spacing|Standing", meta=(ClampMin="1"))
	float StandingMaxMeleeDistance = 140.f;
	UFUNCTION(BlueprintPure, Category="Combat|Melee Spacing")
	void GetMeleeRangeForTarget(const AActor* Target, float& Minimum, float& Preferred, float& Maximum) const;

	//Combat Interface
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag) override;
	virtual void Die(const FVector& DeathImpulse) override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnHealthChange;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMaxHealthChange;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void FreezeTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	bool bFrozen = false;
	
	UFUNCTION(BlueprintCallable)
	void SetToStunned(bool bIsStunned);

	UFUNCTION(BlueprintCallable)
	void SetToEndStun();
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bStunned = false;

	UFUNCTION(BlueprintImplementableEvent)
	void DissolveMesh();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	AActor* CombatTarget;
	
protected:
	
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeAttributes() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* HealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USkeletalMeshComponent* WeaponRH;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USkeletalMeshComponent* WeaponLH;
	
	//Weapon Damage Socket
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponDamageSocket;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UBehaviorTree* BehaviorTree;

	UPROPERTY()
	ARAIController* AIC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FText BossName;

	UFUNCTION(BlueprintImplementableEvent)
	void HandleDroppedItems();
	
};
