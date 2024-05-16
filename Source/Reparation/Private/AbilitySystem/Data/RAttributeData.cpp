// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/RAttributeData.h"

FAttributeData URAttributeData::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound /*= false*/) const
{
	for (const FAttributeData& Info : AttributeData)
	{
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return Info;
		}
	}
	if (bLogNotFound)
	{
		//UE_LOG(LogAO, Error, TEXT("Can't find AttributeData [%s] for AttributeTag [%s]."), *AttributeTag.ToString(), *GetNameSafe(this));
	}
	return FAttributeData();
}
