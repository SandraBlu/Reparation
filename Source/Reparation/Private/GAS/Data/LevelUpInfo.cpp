// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		if (LevelUpInfo.Num() -1 <= Level) return Level;
		if (XP >= LevelUpInfo[Level].LevelUpRequirement)
		{
			++Level;
		}
		else
		{
			bSearching = false;
		}
	}
	return Level;
}
