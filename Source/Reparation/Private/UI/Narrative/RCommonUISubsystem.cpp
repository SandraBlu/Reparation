// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Narrative/RCommonUISubsystem.h"

#include "UI/GAS/RUserWidget.h"

void URCommonUISubsystem::ShowNotification(const FText& NotificationText, const float Duration)
{
	if (CommonHUD && !NotificationText.IsEmptyOrWhitespace())
	{
		CommonHUD->ShowNotification(NotificationText, Duration);
	}
}
