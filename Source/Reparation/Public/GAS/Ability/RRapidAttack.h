#pragma once

#include "GAS/Ability/RChainAttack.h"
#include "RRapidAttack.generated.h"

/** A complete light/heavy combo montage played faster; authored windows own all contact. */
UCLASS(HideCategories=("Melee|Heavy Strike"))
class REPARATION_API URRapidAttack : public URChainAttack
{
    GENERATED_BODY()
public:
    URRapidAttack();
protected:
    friend class FAdvancedMeleeSkillsTest;
    virtual FGameplayTag GetRequiredSkillTag() const override;
};
