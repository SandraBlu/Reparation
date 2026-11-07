#include "GAS/Ability/RQuiet.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(QuietSkill,"ability.stealth.quiet");
URQuiet::URQuiet() { SetAssetTags(FGameplayTagContainer(QuietSkill)); }
float URQuiet::GetHearingRangeMultiplier(int32 Rank) const
{
    if (Rank<=0 || HearingRangeMultipliers.IsEmpty()) return 1.f;
    const int32 Index=FMath::Min(FMath::Min(Rank,3)-1,HearingRangeMultipliers.Num()-1);
    return FMath::Clamp(HearingRangeMultipliers[Index],0.f,1.f);
}
FString URQuiet::GetDescription(int32 Level)
{
    return FString::Printf(TEXT("<Title>Quiet</>\n\n<Default>Reduces the distance at which enemies investigate your movement noise by %.0f%%. Always applies once learned; no loadout slot needed. Explosions and distractions remain fully audible.</>"),100.f*(1.f-GetHearingRangeMultiplier(Level)));
}
