// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/LevelUpInfo.h"
#include "RTypes/REnumTypes.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	while (Level < GetMaxLevel() && LevelUpInfo.IsValidIndex(Level) && FMath::Max(0, XP) >= LevelUpInfo[Level].LevelUpRequirement) ++Level;
	return Level;
}

float ULevelUpInfo::GetProgressForXP(int32 XP) const
{
	if (LevelUpInfo.Num() < 2) return 0.f;
	const int32 Level = FindLevelForXP(XP);
	if (Level >= GetMaxLevel()) return 1.f;
	const int32 Previous = LevelUpInfo[Level - 1].LevelUpRequirement;
	const int32 Next = LevelUpInfo[Level].LevelUpRequirement;
	return Next > Previous ? FMath::Clamp(static_cast<float>(static_cast<int64>(XP) - Previous) / (static_cast<int64>(Next) - Previous), 0.f, 1.f) : 0.f;
}

int32 ULevelUpInfo::GetAttributeReward(int32 FromLevel) const
{
	return FromLevel >= 1 && FromLevel < GetMaxLevel() && LevelUpInfo.IsValidIndex(FromLevel) ? FMath::Max(0, LevelUpInfo[FromLevel].AttributePointReward) : 0;
}

int32 ULevelUpInfo::GetAbilityReward(int32 FromLevel) const
{
	return FromLevel >= 1 && FromLevel < GetMaxLevel() && LevelUpInfo.IsValidIndex(FromLevel) ? FMath::Max(0, LevelUpInfo[FromLevel].AbilityPointReward) : 0;
}

int32 ULevelUpInfo::GetItemXP(ERXPSource Source, int32 Chapter) const
{
	const FRSourceXP* Row = SourceXP.FindByPredicate([&](const FRSourceXP& R){ return R.Source == Source; });
	return (Row && Row->XPByChapter.IsValidIndex(Chapter - 1)) ? Row->XPByChapter[Chapter - 1] : 0;
}

float ULevelUpInfo::GetCatchUpMultiplier(int32 Chapter, int32 CurrentLevel) const
{
	if (Chapter <= 1 || !ChapterEndLevels.IsValidIndex(Chapter - 2)) return 1.f;
	const int32 Gap = ChapterEndLevels[Chapter - 2] - CurrentLevel;
	return 1.f + FMath::Min(CatchUpCap, CatchUpPerLevel * FMath::Max(0, Gap - CatchUpTrigger));
}

int32 ULevelUpInfo::GetAwardedXP(ERXPSource Source, int32 Chapter, int32 CurrentLevel) const
{
	const FRSourceXP* Row = SourceXP.FindByPredicate([&](const FRSourceXP& R){ return R.Source == Source; });
	if (!Row || !Row->XPByChapter.IsValidIndex(Chapter - 1)) return 0;
	const float Mult = Row->bCatchUpEligible ? GetCatchUpMultiplier(Chapter, CurrentLevel) : 1.f;
	return FMath::RoundToInt(Row->XPByChapter[Chapter - 1] * Mult);
}

int32 ULevelUpInfo::GetXPIntoLevel(int32 XP) const
{
	if (LevelUpInfo.Num() < 2) return 0;
	const int32 Level = FindLevelForXP(XP);
	return FMath::Max(0, XP) - LevelUpInfo[Level - 1].LevelUpRequirement;
}

int32 ULevelUpInfo::GetXPToNextLevel(int32 XP) const
{
	if (LevelUpInfo.Num() < 2) return 0;
	const int32 Level = FindLevelForXP(XP);
	if (Level >= GetMaxLevel()) return 0;
	return LevelUpInfo[Level].LevelUpRequirement - LevelUpInfo[Level - 1].LevelUpRequirement;
}
