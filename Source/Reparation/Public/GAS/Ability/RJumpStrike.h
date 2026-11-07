#pragma once
#include "GAS/Ability/RHeavyStrike.h"
#include "RJumpStrike.generated.h"

/** Authored grounded jump attacks, distinct from airborne attack techniques. */
UCLASS(HideCategories=("Melee|Heavy Strike"))
class REPARATION_API URJumpStrike : public URHeavyStrike
{
    GENERATED_BODY()
public:
    URJumpStrike();
    virtual bool ShouldUseThrust() const override { return false; }
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr,const FGameplayTagContainer* Target=nullptr,FGameplayTagContainer* Relevant=nullptr) const override;
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override { return GetDescription(Level); }
    /** Enemy stagger duration, configured in GA_JumpStrike. Set to cover the desired reaction. */
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Melee|Jump Strike",meta=(ClampMin="0",ClampMax="30")) float StaggerSeconds=.65f;
protected:
    friend class FJumpStrikeTest;
    virtual void ProcessMeleeHit(const FHitResult& Hit) override;
    virtual FGameplayTag GetRequiredSkillTag() const override;
};
