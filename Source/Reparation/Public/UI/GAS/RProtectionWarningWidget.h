#pragma once
#include "Blueprint/UserWidget.h"
#include "RProtectionWarningWidget.generated.h"
UCLASS()
class REPARATION_API URProtectionWarningWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    FText Warning;
    virtual TSharedRef<SWidget> RebuildWidget() override;
};
