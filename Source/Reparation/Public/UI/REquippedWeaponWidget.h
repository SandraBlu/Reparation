#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "REquippedWeaponWidget.generated.h"

class UREquipmentComponent;
class UTextBlock;
class UImage;

/** Native UMG panel; can also be subclassed by a Widget Blueprint. */
UCLASS(BlueprintType, Blueprintable)
class REPARATION_API UREquippedWeaponWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintSetter, Category = "Weapon")
	void SetEquipment(UREquipmentComponent* InEquipment);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UPROPERTY(Transient, BlueprintReadWrite, BlueprintSetter = SetEquipment, Category = "Weapon")
	TObjectPtr<UREquipmentComponent> Equipment;

	// Add designer widgets with these names to reuse the native bindings.
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Weapon|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponName;
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Weapon|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponState;
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Weapon|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UImage> WeaponIcon;
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Weapon|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UImage> ToggleButton;
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Weapon|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UImage> SwitchButton;
	UFUNCTION(BlueprintCallable, Category = "Weapon") void RefreshWeapon();
};
