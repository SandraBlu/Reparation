// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/RAbilityInfo.h"

FAbilityData URAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound /*= false*/) const
{
	for (const FAbilityData& Info : AbilityInformation)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}
	if (bLogNotFound)
	{
		//UE_LOG(LogAO, Error, TEXT("Can't find info for AbilityTag[%s] on AbilityInfo [%s]"), *AbilityTag.ToString(), *GetNameSafe(this));
	}
	return FAbilityData();
}
