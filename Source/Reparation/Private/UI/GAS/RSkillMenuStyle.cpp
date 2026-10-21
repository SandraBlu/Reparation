#include "UI/GAS/RSkillMenuStyle.h"
#include "Styling/CoreStyle.h"

FRSkillMenuButtonAppearance::FRSkillMenuButtonAppearance()
{
    Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
    SelectedStyle = Style;
    Font = FCoreStyle::GetDefaultFontStyle("Regular", 16);
}

URSkillMenuStyle::URSkillMenuStyle()
{
    HeaderFont = FCoreStyle::GetDefaultFontStyle("Regular", 20);
    BodyFont = FCoreStyle::GetDefaultFontStyle("Regular", 17);
    BackgroundBrush = *FCoreStyle::Get().GetBrush("WhiteBrush");
    DetailsBrush = BackgroundBrush;
}

const FRSkillMenuButtonAppearance& URSkillMenuStyle::GetButtonAppearance(ERSkillMenuButtonRole Role) const
{
    if (Role == ERSkillMenuButtonRole::Navigation) return NavigationButtons;
    if (Role == ERSkillMenuButtonRole::Action) return ActionButtons;
    return SkillButtons;
}
