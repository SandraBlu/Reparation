// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/LevelUpInfo.h"

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
