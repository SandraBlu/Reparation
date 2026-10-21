#pragma once

#include "GAS/Ability/RMeleeAbility.h"
#include "RHeavyStrike.generated.h"

/** A single random slash or forward thrust. Blueprint defaults own the montage choices. */
UCLASS()
class REPARATION_API URHeavyStrike : public URMeleeAbility
{
    GENERATED_BODY()
public:
    URHeavyStrike();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr, const FGameplayTagContainer* Target=nullptr, FGameplayTagContainer* Relevant=nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Event) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, bool bReplicate, bool bCancelled) override;
    virtual TArray<int32> GetAvailableAttackIndices() const override;
    virtual FRMeleeAttack GetCurrentAttack() const override;
    UFUNCTION(BlueprintPure, Category="Melee|Heavy Strike") virtual bool ShouldUseThrust() const;
protected:
    virtual FGameplayTag GetRequiredSkillTag() const;
    friend class FHeavyStrikeTest;
    /** Attacks (inherited) holds heavy slashes; this separate array holds forward thrusts. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Heavy Strike") TArray<FRMeleeAttack> ThrustAttacks;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Attack", meta=(ClampMin="0.1")) float PlayRate = 1.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Attack", meta=(ClampMin="0")) float DamageMultiplier = 1.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Heavy Strike", meta=(ClampMin="0")) float MinimumForwardSpeed = 30.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Heavy Strike", meta=(ClampMin="0",ClampMax="1")) float ForwardDirectionThreshold = .5f;
    virtual void ProcessMeleeHit(const FHitResult& Hit) override;
    UFUNCTION() void OpenHitWindow(FGameplayEventData Payload);
    UFUNCTION() void CloseHitWindow(FGameplayEventData Payload);
    UFUNCTION() void Completed();
    UFUNCTION() void Interrupted();
private:
    // Latched once at activation, so changing movement cannot change an ongoing attack.
    bool bThrustAttack = false;
};
