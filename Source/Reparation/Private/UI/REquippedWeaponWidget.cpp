#include "UI/REquippedWeaponWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
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
		Panel->AddChildToVerticalBox(WeaponIcon);
		WeaponName = WidgetTree->ConstructWidget<UTextBlock>();
		Panel->AddChildToVerticalBox(WeaponName);
		WeaponState = WidgetTree->ConstructWidget<UTextBlock>();
		Panel->AddChildToVerticalBox(WeaponState);
		ToggleButton = WidgetTree->ConstructWidget<UButton>();
		UTextBlock* ToggleLabel = WidgetTree->ConstructWidget<UTextBlock>();
		ToggleLabel->SetText(NSLOCTEXT("WeaponUI", "Toggle", "Draw / Holster"));
		ToggleButton->SetContent(ToggleLabel);
		Panel->AddChildToVerticalBox(ToggleButton);
		SwitchButton = WidgetTree->ConstructWidget<UButton>();
		UTextBlock* SwitchLabel = WidgetTree->ConstructWidget<UTextBlock>();
		SwitchLabel->SetText(NSLOCTEXT("WeaponUI", "Switch", "Next weapon"));
		SwitchButton->SetContent(SwitchLabel);
		Panel->AddChildToVerticalBox(SwitchButton);
	}
	return Super::RebuildWidget();
}

void UREquippedWeaponWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ToggleButton) ToggleButton->OnClicked.AddUniqueDynamic(this, &ThisClass::ToggleClicked);
	if (SwitchButton) SwitchButton->OnClicked.AddUniqueDynamic(this, &ThisClass::SwitchClicked);
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
	if (ToggleButton) ToggleButton->OnClicked.RemoveDynamic(this, &ThisClass::ToggleClicked);
	if (SwitchButton) SwitchButton->OnClicked.RemoveDynamic(this, &ThisClass::SwitchClicked);
	Super::NativeDestruct();
}

void UREquippedWeaponWidget::RefreshWeapon()
{
	UREquipItem* Item = Equipment ? Equipment->GetEquippedItemAtSlot(EEquipSlot::ES_Weapon) : nullptr;
	if (WeaponName) WeaponName->SetText(Item ? Item->DisplayName : NSLOCTEXT("WeaponUI", "Empty", "No weapon equipped"));
	if (WeaponState) WeaponState->SetText(Equipment && Equipment->IsWeaponArmed()
		? NSLOCTEXT("WeaponUI", "Armed", "Armed") : NSLOCTEXT("WeaponUI", "Holstered", "Holstered"));
	if (WeaponIcon)
	{
		UTexture2D* Icon = Item ? Item->Icon.LoadSynchronous() : nullptr;
		WeaponIcon->SetBrushFromTexture(Icon);
		WeaponIcon->SetDesiredSizeOverride(FVector2D(64.f));
		WeaponIcon->SetVisibility(Icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UREquippedWeaponWidget::ToggleClicked() { if (Equipment) Equipment->ToggleWeapon(); }
void UREquippedWeaponWidget::SwitchClicked() { if (Equipment) Equipment->SwitchWeapon(); }
