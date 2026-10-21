#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateTypes.h"
#include "RSkillMenuStyle.generated.h"

enum class ERSkillMenuButtonRole : uint8 { Navigation, Skill, Action };

USTRUCT(BlueprintType)
struct REPARATION_API FRSkillMenuButtonAppearance
{

    GENERATED_BODY()
    FRSkillMenuButtonAppearance();

    /** Expand Normal/Hovered/Pressed/Disabled to assign textures or materials, including HQUI art. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FButtonStyle Style;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FButtonStyle SelectedStyle;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FSlateFontInfo Font;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FLinearColor TextColor = FLinearColor(0.88f, 0.94f, 1.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FLinearColor SelectedTextColor = FLinearColor::White;
    /** White preserves the colours of assigned textures. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FLinearColor Tint = FLinearColor(0.06f, 0.13f, 0.22f, 1.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FLinearColor SelectedTint = FLinearColor(0.12f, 0.36f, 0.48f, 1.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FMargin TextPadding = FMargin(14.f, 9.f);
};

/** Shared appearance for the native skill views. Edit the asset, then restart PIE to preview. */
UCLASS(BlueprintType)
class REPARATION_API URSkillMenuStyle : public UDataAsset
{
    GENERATED_BODY()
public:
    URSkillMenuStyle();
    const FRSkillMenuButtonAppearance& GetButtonAppearance(ERSkillMenuButtonRole Role) const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FRSkillMenuButtonAppearance NavigationButtons;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FRSkillMenuButtonAppearance SkillButtons;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buttons") FRSkillMenuButtonAppearance ActionButtons;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panels") FSlateBrush BackgroundBrush;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panels") FSlateBrush DetailsBrush;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panels") FLinearColor BackgroundTint = FLinearColor(0.015f, 0.028f, 0.055f, 1.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panels") FLinearColor DetailsTint = FLinearColor(0.025f, 0.05f, 0.09f, 1.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Text") FSlateFontInfo HeaderFont;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Text") FSlateFontInfo BodyFont;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Text") FLinearColor HeaderColor = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Text") FLinearColor BodyColor = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout") FMargin ContentPadding = FMargin(20.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout") FMargin DetailsPadding = FMargin(18.f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout", meta=(ClampMin="0", ClampMax="32")) float ControlSpacing = 6.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout", meta=(ClampMin="0", ClampMax="64")) float ColumnSpacing = 20.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout", meta=(ClampMin="0.2", ClampMax="0.6")) float SkillListWidthFraction = 0.38f;
};
