#pragma once
#include "Abilities/GameplayAbility.h"
#include "RCompanionInterrupt.generated.h"
/** A short counter interrupt, followed by the companion's ordinary attack montage. */
UCLASS()
class REPARATION_API URCompanionInterrupt : public UGameplayAbility
{
    GENERATED_BODY()
public:
    URCompanionInterrupt();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
        FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
