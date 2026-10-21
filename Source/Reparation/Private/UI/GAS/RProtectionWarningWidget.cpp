#include "UI/GAS/RProtectionWarningWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
TSharedRef<SWidget> URProtectionWarningWidget::RebuildWidget()
{
    if (!WidgetTree->RootWidget)
    {
        UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
        Label->SetText(Warning);
        Label->SetJustification(ETextJustify::Center);
        Label->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.75f, 0.25f)));
        WidgetTree->RootWidget = Label;
    }
    return Super::RebuildWidget();
}
