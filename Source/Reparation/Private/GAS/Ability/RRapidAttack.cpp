#include "GAS/Ability/RRapidAttack.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(RapidAttackSkill, "ability.technique.rapidAttack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(RapidAttackCooldown, "cooldown.rapidAttack");

URRapidAttack::URRapidAttack()
{
    SetAssetTags(FGameplayTagContainer(RapidAttackSkill));
    PlayRate = 1.35f;
    DamageMultiplier = 1.f;
}

FGameplayTag URRapidAttack::GetRequiredSkillTag() const { return RapidAttackSkill; }

