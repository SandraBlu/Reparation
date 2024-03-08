// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Data/AttributeInfo.h"

FAttributeData UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	for (const FAttributeData& Info : AttributeInfo)
	{
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return Info;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AttributeInfo [%s] for AttributeTag [%s]."), *AttributeTag.ToString(), *GetNameSafe(this));
	}
	return FAttributeData();
}
