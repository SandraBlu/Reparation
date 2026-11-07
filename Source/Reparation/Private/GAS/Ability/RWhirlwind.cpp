#include "GAS/Ability/RWhirlwind.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(WhirlwindSkill, "ability.technique.whirlwind");
UE_DEFINE_GAMEPLAY_TAG_STATIC(WhirlwindCooldown, "cooldown.whirlwind");

URWhirlwind::URWhirlwind()
{
    SetAssetTags(FGameplayTagContainer(WhirlwindSkill));
    // One hit per target across the spin, even if the montage contains multiple windows.
    bResetHitsPerDamageWindow = false;
    PlayRate = 1.f;
    DamageMultiplier = 1.5f;
}

FGameplayTag URWhirlwind::GetRequiredSkillTag() const { return WhirlwindSkill; }
