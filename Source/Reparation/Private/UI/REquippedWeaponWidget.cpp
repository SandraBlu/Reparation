#include "UI/REquippedWeaponWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/REquipmentComponent.h"
#include "Engine/Texture2D.h"

TSharedRef<SWidget> UREquippedWeaponWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
		Border->SetPadding(FMargin(12.f));
		Border->SetBrushColor(FLinearColor(0.03f, 0.03f, 0.03f, 0.85f));
		WidgetTree->RootWidget = Border;
		UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>();
		Border->SetContent(Panel);
		WeaponIcon = WidgetTree->ConstructWidget<UImage>();
		WeaponIcon->SetDesiredSizeOverride(FVector2D(64.f));
		Panel->AddChildToVerticalBox(WeaponIcon);
		WeaponName = WidgetTree->ConstructWidget<UTextBlock>();
		Panel->AddChildToVerticalBox(WeaponName);
		WeaponState = WidgetTree->ConstructWidget<UTextBlock>();
		Panel->AddChildToVerticalBox(WeaponState);
		ToggleButton = WidgetTree->ConstructWidget<UImage>();
		ToggleButton->SetDesiredSizeOverride(FVector2D(32.f));
		ToggleButton->SetVisibility(ESlateVisibility::HitTestInvisible);
		Panel->AddChildToVerticalBox(ToggleButton);
		SwitchButton = WidgetTree->ConstructWidget<UImage>();
		SwitchButton->SetDesiredSizeOverride(FVector2D(32.f));
		SwitchButton->SetVisibility(ESlateVisibility::HitTestInvisible);
		Panel->AddChildToVerticalBox(SwitchButton);
	}
	return Super::RebuildWidget();
}

void UREquippedWeaponWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Equipment) Equipment->OnWeaponChanged.AddUniqueDynamic(this, &ThisClass::RefreshWeapon);
	RefreshWeapon();
}

void UREquippedWeaponWidget::SetEquipment(UREquipmentComponent* InEquipment)
{
	if (Equipment) Equipment->OnWeaponChanged.RemoveDynamic(this, &ThisClass::RefreshWeapon);
	Equipment = InEquipment;
	if (Equipment) Equipment->OnWeaponChanged.AddUniqueDynamic(this, &ThisClass::RefreshWeapon);
	RefreshWeapon();
}


void UREquippedWeaponWidget::NativeDestruct()
{
	if (Equipment) Equipment->OnWeaponChanged.RemoveDynamic(this, &ThisClass::RefreshWeapon);
	Super::NativeDestruct();
}

void UREquippedWeaponWidget::RefreshWeapon()
{
	UREquipItem* Item = Equipment ? Equipment->GetEquippedItemAtSlot(EEquipSlot::ES_Weapon) : nullptr;
	if (WeaponName) WeaponName->SetText(Item ? Item->DisplayName : NSLOCTEXT("WeaponUI", "Empty", "No weapon"));
	if (WeaponState) WeaponState->SetText(Equipment && Equipment->IsWeaponArmed()
		? NSLOCTEXT("WeaponUI", "Armed", "Armed") : NSLOCTEXT("WeaponUI", "Disarmed", "Disarmed"));
	if (WeaponIcon)
	{
		UTexture2D* Icon = Item ? Item->Icon.LoadSynchronous() : nullptr;
		WeaponIcon->SetBrushFromTexture(Icon);
		WeaponIcon->SetVisibility(Icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
