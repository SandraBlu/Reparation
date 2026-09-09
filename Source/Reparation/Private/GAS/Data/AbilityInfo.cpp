// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/AbilityInfo.h"

FRAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FRAbilityInfo& Info : AbilityInfo)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find infofor AbilityTag[%s] on AblilityInfo [%s]"), *AbilityTag.ToString(), *GetNameSafe(this));
	}
	return FRAbilityInfo();
}
