#pragma once
#include "GAS/Ability/RMeleeAbility.h"
#include "RPlayerMeleeSkills.generated.h"

UCLASS()
class REPARATION_API URHeavyStrikeTechnique : public UGameplayAbility
{
    GENERATED_BODY()
public:
    URHeavyStrikeTechnique();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr, const FGameplayTagContainer* Target=nullptr, FGameplayTagContainer* Relevant=nullptr) const override { return false; }
};
UCLASS()
class REPARATION_API URChainAttackTechnique : public URHeavyStrikeTechnique
{
    GENERATED_BODY()
public: URChainAttackTechnique();
};

/** Counter uses existing weapon windows; shield bash uses the player's existing magic-shield guard. */
UCLASS()
class REPARATION_API URPlayerMeleeFollowup : public URMeleeAbility
{
    GENERATED_BODY()
public:
    URPlayerMeleeFollowup();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr, const FGameplayTagContainer* Target=nullptr, FGameplayTagContainer* Relevant=nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Event) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, bool bReplicate, bool bCancelled) override;
    UFUNCTION(BlueprintPure, Category="Melee") bool CanHitFollowupTarget(AActor* Target) const;
protected:
    friend class FPlayerMeleeSkillsTest;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Followup") bool bShieldBash = false;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Followup", meta=(ClampMin="1")) float Reach = 240.f;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Followup", meta=(ClampMin="0.1")) float CooldownSeconds = 5.f;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Followup", meta=(ClampMin="0")) float DamageMultiplier = 1.5f;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Shield Bash", meta=(ClampMin="0")) float StunSeconds = .6f;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Shield Bash", meta=(ClampMin="0.01")) float BashImpactTime = .2f;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Shield Bash", meta=(ClampMin="0.02")) float BashRecoveryTime = .6f;
    virtual void ProcessMeleeHit(const FHitResult& Hit) override;
    UFUNCTION() void OpenHitWindow(FGameplayEventData Payload);
    UFUNCTION() void CloseHitWindow(FGameplayEventData Payload);
    UFUNCTION() void Completed();
    UFUNCTION() void Interrupted();
    void ImpactBash();
    void DamageTarget(AActor* Target);
    FTimerHandle BashImpactTimer, BashRecoveryTimer;
    TWeakObjectPtr<AActor> CounterTarget;
    bool bCounterHit = false;
};

UCLASS()
class REPARATION_API URPlayerCounterstrike : public URPlayerMeleeFollowup
{ GENERATED_BODY() public: URPlayerCounterstrike(); };
UCLASS()
class REPARATION_API URPlayerShieldBash : public URPlayerMeleeFollowup
{ GENERATED_BODY() public: URPlayerShieldBash(); };
