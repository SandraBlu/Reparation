#pragma once

#include "GAS/Ability/RHeavyStrike.h"
#include "RChainAttack.generated.h"

/** Full-combo montage alternatives using the shared committed melee execution. */
UCLASS(HideCategories=("Melee|Heavy Strike"))
class REPARATION_API URChainAttack : public URHeavyStrike
{
    GENERATED_BODY()
public:
    URChainAttack();
    virtual bool ShouldUseThrust() const override { return false; }
protected:
    virtual FGameplayTag GetRequiredSkillTag() const override;
};
