// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/RInputConfig.h"

const UInputAction* URInputConfig::FindAbilityInputForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FRInputAction& Action : AbilityInput)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("No AbilityInput for InputTag [%s], on InputConfig [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}
	return nullptr;
}
