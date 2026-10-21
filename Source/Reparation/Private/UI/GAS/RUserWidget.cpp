// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GAS/RUserWidget.h"

#include "UI/Narrative/RCommonUISubsystem.h"
#include "UI/GAS/Controllers/RAbilityMenuController.h"
#include "UI/GAS/RSkillBrowser.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

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
	if (GetClass()->GetName() == TEXT("UI_AbilityMenu_C"))
	{
		if (URAbilityMenuController* Menu = Cast<URAbilityMenuController>(InWidgetController))
		{
			if (Menu->SkillMenuHost == this && Menu->SkillMenuCanvas) return;
			// A recreated menu must not reuse children from the previous widget instance.
			Menu->SkillBrowser = nullptr;
			Menu->LegacySkillMenuWidgets.Reset();
			Menu->LegacySkillMenuVisibilities.Reset();
			Menu->SkillMenuHost = this;
			UCanvasPanel* Canvas = Cast<UCanvasPanel>(GetRootWidget());
			if (!Canvas)
			{
				if (UOverlay* Overlay = Cast<UOverlay>(GetRootWidget()))
				{
					// Preserve the authored background/frame. Switch only the original content.
					if (UWidget* Content = WidgetTree->FindWidget(TEXT("SizeBox_root")))
					{
						Menu->LegacySkillMenuWidgets.Add(Content);
						Menu->LegacySkillMenuVisibilities.Add(Content->GetVisibility());
					}
					Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
					Canvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
					UOverlaySlot* Layer = Overlay->AddChildToOverlay(Canvas);
					Layer->SetHorizontalAlignment(HAlign_Fill);
					Layer->SetVerticalAlignment(VAlign_Fill);
				}
			}
			if (Canvas)
			{
				Menu->SkillMenuCanvas = Canvas;
				// The skill tree is now the only skill page. Retain the existing outer menu frame.
				Menu->LearnViewButton = nullptr;
				Menu->LoadoutViewButton = nullptr;
				Menu->OpenSkillBrowser();
			}
		}
	}
}
