// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/RUserWidget.h"

#include "UI/Narrative/RCommonUISubsystem.h"

void URUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//Store the HUD in a subsystem - that way, 
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (URCommonUISubsystem* NSS = GI->GetSubsystem<URCommonUISubsystem>())
			{
				NSS->CommonHUD = this;
			}
		}
	}
	
}

void URUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
