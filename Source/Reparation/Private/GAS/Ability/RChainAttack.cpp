#include "GAS/Ability/RChainAttack.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(ChainAttackSkill, "ability.technique.chainAttack");

URChainAttack::URChainAttack()
{
    SetAssetTags(FGameplayTagContainer(ChainAttackSkill));
    bResetHitsPerDamageWindow = true;
    // Provisional per-contact values; editable on GA_ChainAttack.
    DamageMultiplier = .75f;
    PlayRate = 1.15f;
}

FGameplayTag URChainAttack::GetRequiredSkillTag() const { return ChainAttackSkill; }
