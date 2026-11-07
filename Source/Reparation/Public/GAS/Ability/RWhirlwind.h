#pragma once

#include "GAS/Ability/RHeavyStrike.h"
#include "RWhirlwind.generated.h"

/** A spinning montage sweeps the weapon through 360 degrees inside authored contact windows. */
UCLASS(HideCategories=("Melee|Heavy Strike"))
class REPARATION_API URWhirlwind : public URHeavyStrike
{
    GENERATED_BODY()
public:
    URWhirlwind();
    virtual bool ShouldUseThrust() const override { return false; }
protected:
    friend class FAdvancedMeleeSkillsTest;
    virtual FGameplayTag GetRequiredSkillTag() const override;
};
