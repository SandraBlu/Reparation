#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "REquippedWeaponWidget.generated.h"

class UREquipmentComponent;
class UTextBlock;
class UButton;
class UImage;

/** Native UMG panel; can also be subclassed by a Widget Blueprint. */
UCLASS()
class REPARATION_API UREquippedWeaponWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetEquipment(UREquipmentComponent* InEquipment);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UPROPERTY(Transient) TObjectPtr<UREquipmentComponent> Equipment;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WeaponName;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WeaponState;
	UPROPERTY(Transient) TObjectPtr<UImage> WeaponIcon;
	UPROPERTY(Transient) TObjectPtr<UButton> ToggleButton;
	UPROPERTY(Transient) TObjectPtr<UButton> SwitchButton;
	UFUNCTION() void RefreshWeapon();
	UFUNCTION() void ToggleClicked();
	UFUNCTION() void SwitchClicked();
};
