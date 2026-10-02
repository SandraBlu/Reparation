// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "TimerManager.h"
#include "GAS/Ability/RDamageAbility.h"
#include "RMeleeAbility.generated.h"

class ARWeapon;
class UAnimMontage;
class UPawnCombatComponent;

/** Ordered attack entry; ability points upgrade the GAS ability level. */
USTRUCT(BlueprintType)
struct FRMeleeAttack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee")
	FName SectionName = NAME_None;

	/** Ignored for entry zero, which always unlocks at level 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (ClampMin = "2"))
	int32 RequiredAbilityLevel = 2;
};

/**
 * Blueprint-driven player melee base using ARWeapon and inherited damage parameters.
 * On activation: CommitAbility, BeginMeleeAttack, then play GetCurrentAttack's montage.
 * Animation notifies control damage/combo windows. The ability runs weapon traces
 * during the damage window and forwards unique hits to OnMeleeHit for Blueprint damage.
 * QueueComboAttack on
 * repeated input; AdvanceCombo at a transition notify, then play the next montage.
 * EndAbility when the animation completes or is interrupted.
 */
UCLASS()
class REPARATION_API URMeleeAbility : public URDamageAbility
{
	GENERATED_BODY()

public:
	URMeleeAbility();

	UFUNCTION(BlueprintPure, Category = "Ability")
	UPawnCombatComponent* GetCombatComponentFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "Melee|Weapon")
	ARWeapon* GetMeleeWeapon() const;

	/** False if the mesh is missing either melee socket. */
	UFUNCTION(BlueprintCallable, Category = "Melee|Weapon")
	bool GetMeleeTraceLocations(FVector& Start, FVector& End) const;

	UFUNCTION(BlueprintPure, Category = "Melee|Combo")
	int32 GetUnlockedAttackCount() const;

	UFUNCTION(BlueprintCallable, Category = "Melee|Combo")
	bool BeginMeleeAttack();

	UFUNCTION(BlueprintCallable, Category = "Melee|Combo")
	bool QueueComboAttack();

	UFUNCTION(BlueprintCallable, Category = "Melee|Combo")
	void SetComboWindowOpen(bool bOpen);

	/** Consumes buffered input and advances without wrapping to the first attack. */
	UFUNCTION(BlueprintCallable, Category = "Melee|Combo")
	bool AdvanceCombo();

	UFUNCTION(BlueprintPure, Category = "Melee|Combo")
	FRMeleeAttack GetCurrentAttack() const;

	/** Starts/stops ability-owned tracing. Call from the window gameplay-event listeners. */
	UFUNCTION(BlueprintCallable, Category = "Melee|Damage")
	void SetDamageWindowOpen(bool bOpen);

	/** Server only. Each target can receive inherited damage once per attack. */
	UFUNCTION(BlueprintCallable, Category = "Melee|Damage")
	bool ApplyMeleeHit(AActor* TargetActor);

	/** Validates and records a target once per swing without applying damage. */
	UFUNCTION(BlueprintCallable, Category = "Melee|Damage")
	bool RegisterMeleeHit(AActor* TargetActor);

	/** Once per target per swing. Build inherited damage parameters and apply them here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Melee|Damage", meta = (DisplayName = "On Melee Hit"))
	void OnMeleeHit(const FHitResult& Hit);

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Trace", meta = (ClampMin = "0.1"))
	float TraceRadius = 10.f;

	/** Seconds between trace samples; socket positions are read anew for every sample. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Trace", meta = (ClampMin = "0.001"))
	float TraceInterval = 0.01f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Trace")
	bool bDrawDebugTrace = false;

	/** Missing or locked entries stop the chain; configure entries in execution order. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Combo")
	TArray<FRMeleeAttack> Attacks;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Melee|Combo")
	int32 CurrentAttackIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Melee|Combo")
	bool bComboWindowOpen = false;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Melee|Combo")
	bool bComboInputQueued = false;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Melee|Damage")
	bool bDamageWindowOpen = false;

private:
	void ResetMeleeState();
	void TraceCurrentAttack();
	FTimerHandle MeleeTraceTimer;

	UPROPERTY(Transient)
	TObjectPtr<ARWeapon> ActiveMeleeWeapon = nullptr;

	TSet<TWeakObjectPtr<AActor>> HitActors;
};
