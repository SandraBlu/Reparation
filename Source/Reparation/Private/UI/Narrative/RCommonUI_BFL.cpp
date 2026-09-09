// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Narrative/RCommonUI_BFL.h"

#include "Kismet/GameplayStatics.h"
#include "UI/Narrative/RCommonUISubsystem.h"

void URCommonUI_BFL::PushHUDNotification(const UObject* WorldContextObject, FText Message, const float Duration)
{
	if (!Message.IsEmptyOrWhitespace())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject))
		{
			if (URCommonUISubsystem* NSS = GI->GetSubsystem<URCommonUISubsystem>())
			{
				NSS->ShowNotification(Message, Duration);
			}
		}
	}
}
