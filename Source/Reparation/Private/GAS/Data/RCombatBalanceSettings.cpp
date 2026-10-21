#include "GAS/Data/RCombatBalanceSettings.h"

int32 FRCombatDifficultyProfile::GetPlayerAttackerLimit(int32 Level) const
{
    int32 Limit = PlayerAttackers, BestLevel = 0;
    for (const FRCombatLevelBand& Band : PlayerLevelBands)
        if (Band.MinimumPlayerLevel >= 1 && Band.MinimumPlayerLevel <= FMath::Max(1, Level) && Band.MinimumPlayerLevel >= BestLevel)
        { BestLevel = Band.MinimumPlayerLevel; Limit = Band.PlayerAttackers; }
    return FMath::Clamp(Limit, 1, 6);
}

URCombatBalanceSettings::URCombatBalanceSettings()
{
    Easy.PlayerAttackers = 1;
    Easy.EnemyDamageMultiplier = 0.85f;
    Hard.PlayerAttackers = 3;
    Hard.CompanionAttackers = 2;
    Hard.EnemyDamageMultiplier = 1.2f;
    VeryHard = Hard;
    VeryHard.EnemyDamageMultiplier = 1.4f;
}

FRCombatDifficultyProfile URCombatBalanceSettings::GetProfile(ERGameDifficulty Difficulty) const
{
    switch (Difficulty)
    {
    case ERGameDifficulty::Easy: return Easy;
    case ERGameDifficulty::Hard: return Hard;
    case ERGameDifficulty::VeryHard: return VeryHard;
    default: return Normal;
    }
}
