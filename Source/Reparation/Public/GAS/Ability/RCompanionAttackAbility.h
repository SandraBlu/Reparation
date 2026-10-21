#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RDamageAbility.h"
#include "RCompanionAttackAbility.generated.h"

class ARCompanion;

/** Single-target montage attack; configure damage and a retargeted montage in Blueprint defaults. */
UCLASS()
class REPARATION_API URCompanionAttackAbility : public URDamageAbility
{
	GENERATED_BODY()
public:
	URCompanionAttackAbility();
	bool IsConfiguredFor(const ARCompanion* Companion) const;
	FText GetConfigurationIssue(const ARCompanion* Companion) const;
	/** Explicit montage avoids legacy companion Blueprint combat-interface overrides. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Companion|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Companion|Attack", meta=(ClampMin="1"))
	float AttackRange = 180.f;
	/** Fraction of the montage at which the single hit is validated and applied. */
	UPROPERTY(EditDefaultsOnly, Category="Companion|Attack", meta=(ClampMin="0.01", ClampMax="0.99"))
	float HitTimeFraction = 0.4f;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	TWeakObjectPtr<AActor> AttackTarget;
	FTimerHandle HitTimer;
	FTimerHandle TimeoutTimer;
	void ApplyHit();
	UAnimMontage* ChooseMontageFor(const ARCompanion* Companion) const;
	UFUNCTION() void CompleteAttack();
	UFUNCTION() void InterruptAttack();
};
