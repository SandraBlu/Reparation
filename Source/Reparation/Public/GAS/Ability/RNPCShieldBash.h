#pragma once
#include "GAS/Ability/RMeleeAbility.h"
#include "RNPCShieldBash.generated.h"

/** NPC-only shield attack. Not part of player skill progression or loadout. */
UCLASS()
class REPARATION_API URNPCShieldBash : public URMeleeAbility
{
    GENERATED_BODY()
public:
    URNPCShieldBash();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr,const FGameplayTagContainer* Target=nullptr,FGameplayTagContainer* Relevant=nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation,bool bReplicate,bool bCancelled) override;
protected:
    UPROPERTY(EditDefaultsOnly,Category="Combat|Shield Bash",meta=(ClampMin="1")) float Reach=240.f;
    UPROPERTY(EditDefaultsOnly,Category="Combat|Shield Bash",meta=(ClampMin="0")) float DamageMultiplier=.65f;
    bool CanHitTarget(AActor* Target) const;
    virtual void ProcessMeleeHit(const FHitResult& Hit) override;
    UFUNCTION() void OpenHitWindow(FGameplayEventData Payload);
    UFUNCTION() void CloseHitWindow(FGameplayEventData Payload);
    UFUNCTION() void Completed();
    UFUNCTION() void Interrupted();
    bool bBashHit=false;
    TWeakObjectPtr<AActor> BashTarget;
};
