#include "GAS/Ability/RElementalMastery.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(SkillFire, "ability.mastery.fire");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SkillIce, "ability.mastery.ice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SkillElectric, "ability.mastery.electric");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SkillPoison, "ability.mastery.poison");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SkillDark, "ability.mastery.dark");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TechniqueType, "ability.type.technique");
UE_DEFINE_GAMEPLAY_TAG_STATIC(SkillAdvancedProjectiles, "ability.technique.advancedProjectiles");
URAdvancedProjectileMastery::URAdvancedProjectileMastery() { SetAssetTags(FGameplayTagContainer(SkillAdvancedProjectiles)); }
URFireMastery::URFireMastery() { SetAssetTags(FGameplayTagContainer(SkillFire)); }
URIceMastery::URIceMastery() { SetAssetTags(FGameplayTagContainer(SkillIce)); }
URElectricMastery::URElectricMastery() { SetAssetTags(FGameplayTagContainer(SkillElectric)); }
URPoisonMastery::URPoisonMastery() { SetAssetTags(FGameplayTagContainer(SkillPoison)); }
URDarkMastery::URDarkMastery() { SetAssetTags(FGameplayTagContainer(SkillDark)); }
FGameplayTag URElementalMastery::GetSkillForDamage(FGameplayTag Damage)
{
    const FString Name = Damage.ToString();
    if (Name == TEXT("damage.elemental.fire")) return SkillFire;
    if (Name == TEXT("damage.elemental.ice")) return SkillIce;
    if (Name == TEXT("damage.elemental.electric")) return SkillElectric;
    if (Name == TEXT("damage.poison")) return SkillPoison;
    if (Name == TEXT("damage.dark")) return SkillDark;
    return FGameplayTag();
}
